/*
 *  Copyright (c) 2010-2013, Rutgers, The State University of New Jersey
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the organization(s) stated above nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/// @cond API_ONLY
/**
 * @file   log.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   June, 2013
 * @brief  Logging methods.
 *
 * Logging methods. Differentiated between UNIX and Android
 */

#ifndef MFLOG_H
#define MFLOG_H

/*! Prints a string on the standard output
 * \param str The string to print. */
void stdLog(char *str);

/*! Prints a string on the standard output and an integer value using printf formatting system
 * \param str The string to print.
 * \param val The integer to print */
void stdLogVal(char *str, int val);

/*! Prints a string on the standard output and an other string value using printf formatting system
 * \param str The string to print.
 * \param val The other string to print */
void stdLogStr(char *str, char *val);

/*! Prints a string on the error output
 * \param str The string to print. */
void errLog(char *str);

/*! Prints a string on the error output and an integer value using printf formatting system
 * \param str The string to print.
 * \param val The integer to print */
void errLogVal(char *str, int val);

/*! Prints a string on the error output and an other string value using printf formatting system
 * \param str The string to print.
 * \param val The other string to print */
void errLogStr(char *str, char *val);

#endif

/// @endcond