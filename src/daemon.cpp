/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include "daemon.hpp"
#include "usb_daemon_conf.h"
#include <iostream>
#include "web_usb_comm.h"
string TDaemon::CONFIG_FILE;
string TDaemon::PID_FILE;
const unsigned int TDaemon::FD_LIMIT=2048;

void TDaemon::SetPidFile(const char* Filename)
//функция создает PID-файл
{
    FILE *f, *exist;
    exist=fopen(Filename,"r");
    int pid_exists=0;
    if(exist==NULL && errno==ENOENT)//file doesn't exist
    {//good
        f = fopen(Filename, "w+");
        if (f)
        {
            fprintf(f, "%u", getpid());
            fclose(f);
        }
        else
        {
            WriteLog("Failed to set PID file: %s",strerror(errno));
        }
    }
    else
    {//file exists. is process run?
        if(fscanf(exist,"%i",&pid_exists)>0)//no errors
        {
            if(kill(pid_exists,0))//Process doesn't exist
            {
                fclose(exist);
                f = fopen(Filename, "w+");
                if (f)
                {
                    fprintf(f, "%u", getpid());
                    fclose(f);
                }
                else
                {
                    WriteLog("Failed to set PID file: %s",strerror(errno));
                }
            }
            else
            {//Process exists
                exit(EPROCESS_EXISTS);
            }
        }
    }
}

int TDaemon::MonitorProc()
//следим за потомком
{
    int      pid;
    int      status;
    int      need_start = 1;
    sigset_t sigset;
    siginfo_t siginfo;
    // настраиваем сигналы которые будем обрабатывать
    sigemptyset(&sigset);
    
    // сигнал остановки процесса пользователем
    sigaddset(&sigset, SIGQUIT);
    
    // сигнал для остановки процесса пользователем с терминала
    sigaddset(&sigset, SIGINT);
    
    // сигнал запроса завершения процесса
    sigaddset(&sigset, SIGTERM);
    
    // сигнал посылаемый при изменении статуса дочернего процесса
    sigaddset(&sigset, SIGCHLD); 
    
    // пользовательский сигнал который мы будем использовать для обновления конфига
    sigaddset(&sigset, SIGHUP); 
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    // данная функция создаст файл с нашим PID'ом
    SetPidFile(PID_FILE.c_str());
    // бесконечный цикл работы
    for (;;)
    {
        // если необходимо создать потомка
        if (need_start)
        {
            // создаём потомка
            pid = fork();
        }
        
        need_start = 1;
        
        if (pid == -1) // если произошла ошибка
        {
            // запишем в лог сообщение об этом
            WriteLog("%s [MONITOR] Fork failed (%s)\n",getTime(), strerror(errno));
        }
        else if (!pid) // если мы потомок
        {
            // данный код выполняется в потомке
            // запустим функцию отвечающую за работу демона
            status = WorkProc();
            // завершим процесс
            exit(status);
        }
        else // если мы родитель
        {
            // данный код выполняется в родителе
            
            // ожидаем поступление сигнала
            sigwaitinfo(&sigset, &siginfo);
            // если пришел сигнал от потомка
            if (siginfo.si_signo == SIGCHLD)
            {
                // получаем статус завершение
                wait(&status);
                
                // преобразуем статус в нормальный вид
                status = WEXITSTATUS(status);

                 // если потомок завершил работу с кодом говорящем о том, что нет нужды дальше работать
                if (status == CHILD_NEED_TERMINATE)
                {
                    // запишем в лог сообщени об этом 
                    WriteLog("%s [MONITOR] Child stopped\n", getTime());
                    
                    // прервем цикл
                    break;
                }
                else if (status == CHILD_NEED_WORK) // если требуется перезапустить потомка
                {
                    // запишем в лог данное событие
                    WriteLog("%s [MONITOR] Child restart\n", getTime());
                }
            }
            else if (siginfo.si_signo == SIGHUP) // если пришел сигнал что необходимо перезагрузить конфиг
            {
                kill(pid, SIGHUP); // перешлем его потомку
                need_start = 0; // установим флаг что нам не надо запускать потомка заново
            }
            else // если пришел какой-либо другой ожидаемый сигнал
            {
                // запишем в лог информацию о пришедшем сигнале
                WriteLog("%s [MONITOR] Signal %s\n", getTime(), strsignal(siginfo.si_signo));
                
                // убьем потомка
                kill(pid, SIGTERM);
                status = 0;
                break;
            }
        }
    }

    // запишем в лог, что мы остановились
    WriteLog("%s [MONITOR] Stop\n", getTime());
    
    // удалим файл с PID'ом
    unlink(PID_FILE.c_str());
    
    return status;
}

void TDaemon::SelfDebug(int sig, siginfo_t *si, void *ptr)
//дебажим свои ошибки
{
    void* ErrorAddr;
    void* Trace[16];
    int    x;
    int    TraceSize;
    char** Messages;

    // запишем в лог что за сигнал пришел
    WriteLog("%s [DAEMON] Signal: %s, Addr: 0x%0.16X\n", getTime(), strsignal(sig), si->si_addr);

    
    #if __WORDSIZE == 64 // если дело имеем с 64 битной ОС
        // получим адрес инструкции которая вызвала ошибку
        ErrorAddr = (void*)((ucontext_t*)ptr)->uc_mcontext.gregs[REG_RIP];
    #else 
        // получим адрес инструкции которая вызвала ошибку
        ErrorAddr = (void*)((ucontext_t*)ptr)->uc_mcontext.gregs[REG_EIP];
    #endif

    // произведем backtrace чтобы получить весь стек вызовов 
    TraceSize = backtrace(Trace, 16);
    Trace[1] = ErrorAddr;

    // получим расшифровку трасировки
    Messages = backtrace_symbols(Trace, TraceSize);
    if (Messages)
    {
        WriteLog("== Backtrace ==\n");
        
        // запишем в лог
        for (x = 1; x < TraceSize; x++)
        {
            WriteLog("%s\n", Messages[x]);
        }
        
        WriteLog("== End Backtrace ==\n");
        free(Messages);
    }
    WriteLog("%s [DAEMON] Stopped\n", getTime());
    
    // остановим все рабочие потоки и корректно закроем всё что надо
    //DestroyWorkThread();
    
    // завершим процесс с кодом требующим перезапуска
    exit(CHILD_NEED_WORK);
}

int TDaemon::SetFdLimit(int MaxFd)
//выставляем максимальное число открытых дескрипторов
{
    struct rlimit lim;
    int          status;

    // зададим текущий лимит на кол-во открытых дискриптеров
    lim.rlim_cur = MaxFd;
    // зададим максимальный лимит на кол-во открытых дискриптеров
    lim.rlim_max = MaxFd;

    // установим указанное кол-во
    status = setrlimit(RLIMIT_NOFILE, &lim);
    
    return status;
}

int TDaemon::ReloadConfig()

{
    return LoadConfig(CONFIG_FILE.c_str());
}

int TDaemon::LoadConfig(const char* Filename)

{
    FILE* f=fopen(Filename,"r");
    
    fclose(f);
    return 0;
}

TDaemon::TDaemon(const char* ConfigName)

{
        // данный код уже выполняется в процессе потомка
        // разрешаем выставлять все биты прав на создаваемые файлы, 
        // иначе у нас могут быть проблемы с правами доступа
        umask(0);
        
        //Задаем путь к конфигу
        CONFIG_FILE=string(ETC_CONF_DIR)+ConfigName;
        //загружаем конфиг
        LoadConfig(CONFIG_FILE.c_str());
        //задаем путь к логу
        SetLogFile((string(LOG_DIR)+PROJ_NAME+".log").c_str());
        //задаем путь к PID-файлу
        PID_FILE=string(RUN_DIR)+PROJ_NAME+".pid";
        
        // создаём новый сеанс, чтобы не зависеть от родителя
        setsid();
        
        // переходим в корень диска, если мы этого не сделаем, то могут быть проблемы.
        // к примеру с размонтированием дисков
        int i=chdir("/");
        if(i<0)
        {
            WriteLog("%s [DAEMON] Failed to set default directory: %s\n", getTime(),strerror(errno));
        }
        // закрываем дискрипторы ввода/вывода/ошибок, так как нам они больше не понадобятся
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
}

int TDaemon::WorkProc()

{
    struct sigaction sigact;
    sigset_t         sigset;
    int             signo;
    int             status;
    // сигналы об ошибках в программе будут обрататывать более тщательно
    // указываем что хотим получать расширенную информацию об ошибках
    sigact.sa_flags = SA_SIGINFO;
    // задаем функцию обработчик сигналов
    sigact.sa_sigaction = SelfDebug;

    sigemptyset(&sigact.sa_mask);

    // установим наш обработчик на сигналы
    sigaction(SIGFPE, &sigact, 0); // ошибка FPU
    sigaction(SIGILL, &sigact, 0); // ошибочная инструкция
    sigaction(SIGSEGV,&sigact, 0); // ошибка доступа к памяти
    sigaction(SIGBUS, &sigact, 0); // ошибка шины, при обращении к физической памяти

    sigemptyset(&sigset);
    
    // блокируем сигналы которые будем ожидать
    // сигнал остановки процесса пользователем
    sigaddset(&sigset, SIGQUIT);
    // сигнал для остановки процесса пользователем с терминала
    sigaddset(&sigset, SIGINT);
    // сигнал запроса завершения процесса
    sigaddset(&sigset, SIGTERM);
    // пользовательский сигнал который мы будем использовать для обновления конфига
    sigaddset(&sigset, SIGHUP); 
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    // Установим максимальное кол-во дискрипторов которое можно открыть
    SetFdLimit(FD_LIMIT);
    // запишем в лог, что наш демон стартовал
    WriteLog("%s [DAEMON] Started\n", getTime());
    DaemonFunction();
        // цикл ожидания сообщений
        while(1)
        {
            // ждем указанных сообщений
            sigwait(&sigset, &signo);
            switch(signo)
            {
                // если это сообщение обновления конфига
                case SIGHUP:
                status = ReloadConfig();
                    if (status != 0)
                    {
                        WriteLog("%s [DAEMON] Reload config failed: %s\n", getTime(),strerror(errno));
                    }
                    else
                    {
                        WriteLog("%s [DAEMON] Reload config OK\n", getTime());
                    }
                break;
                
                case SIGQUIT:
                case SIGINT:
                case SIGTERM:
                default:
                    WriteLog("%s [DAEMON] Received signal %d\n", getTime(),signo);
                    WriteLog("%s [DAEMON] Stopped\n", getTime());
                    return CHILD_NEED_TERMINATE;
            }
        }
    WriteLog("%s [DAEMON] Stopped\n", getTime());
    
    // вернем код не требующим перезапуска
    return CHILD_NEED_TERMINATE;
}
