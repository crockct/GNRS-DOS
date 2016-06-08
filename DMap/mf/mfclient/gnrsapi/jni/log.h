/**
 *
 * User: wontoniii
 * Date: 6/13/13
 * File: log.h
 * Description: logging methods. Differentiated between UNIX and Android
 */

#ifndef MFLOG_H
#define MFLOG_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

void stdLog(const char *str);

void stdLogVal(const char *str, int val);

void stdLogStr(const char *str, const char *val);

void errLog(const char *str);

void errLogVal(const char *str, int val);

void errLogStr(const char *str, const char *val);

#endif
