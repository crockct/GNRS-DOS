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

/**
 * @file   errors.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   June, 2013
 * @brief  Define possible errors occurring in MobilityFirst's API and stack.
 *
 * Define possible errors occurring in MobilityFirst's API and stack.
 */

#ifndef errors_H_
#define errors_H_

/*! \def WRONG_MESSAGE
 * \brief Wrong message from api
 */
#define WRONG_MESSAGE -1

/*! \def SOCKET_ERROR
 * \brief Wrong message from api
 */
#define SOCKET_ERROR -2

/*! \def SEMAPHORE_ERROR
 * \brief Wrong message from api
 */
#define SEMAPHORE_ERROR -3

/*! \def THREAD_ERROR
 * \brief Wrong message from api
 */
#define THREAD_ERROR -4

/*! \def PARAM_ERROR
 * \brief Wrong message from api
 */
#define PARAM_ERROR -5

/*! \def ATTACH_ERROR
 * \brief Wrong message from api
 */
#define ATTACH_ERROR -6

/*! \def DETACH_ERROR
 * \brief Wrong message from api
 */
#define DETACH_ERROR -7

/*! \def CLOSED_ERROR
 * \brief The socket has been closed
 */
#define CLOSED_ERROR -8

/*! \def OPERATION_NOT_PERMITTED
 * \brief The operation is not permitted on the socket
 */
#define OPERATION_NOT_PERMITTED -9

/*! \def BUFFER_ERROR
 * \brief Beffer transfer failed
 */
#define BUFFER_ERROR -10

#endif /* errors_H_ */
