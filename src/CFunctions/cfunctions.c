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

#include "cfunctions.h"

static char Time[10];
static char *LOG_FILE;

void SetLogFile(const char* filename)
//функция устанавливает имя файла логгирования
{
	if (LOG_FILE)
		free(LOG_FILE);
	LOG_FILE=strndup(filename,128);
}

char* GetLogFile()
//функция возвращает текущее имя файла логгирования
{
	return LOG_FILE;
}

void FreeLogFile()
//функция очищает переменную файла логгирования
{
	if(LOG_FILE)
	{
		free(LOG_FILE);
		LOG_FILE = NULL;
	}
}
char* getTime()
//функция возвращает форматированное время
{ 
    bzero(Time, 10);
    time_t seconds = time(NULL);
    struct tm* timeinfo = localtime(&seconds);
    strftime(Time, 9, "%T",timeinfo);
    return Time;
}

void WriteLog(const char* format, ...)
//функция пишет в лог
{
    FILE * lf;
    lf=fopen(LOG_FILE,"a");
    va_list args;
    va_start (args, format);
    vfprintf(lf, format, args); //печатаем в файл
    va_end (args);
    fclose(lf);
}
