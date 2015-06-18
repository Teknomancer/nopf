/* $Id: Errors.h 192 2014-05-14 06:52:35Z marshan $ */
/** @file
 * Error codes and routines header.
 */

/*
 * Copyright (C) 2011 Ramshankar (aka Teknomancer)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ERRORS_H___
#define ERRORS_H__

/*
 * We use a sed script to convert symbolic names from numerics, marked by the sed _BEGIN & _END markers.
 * Each error code needs to be defined with a description before it in the form of a doxygen comment.
 */
/* ERR_SED_BEGIN */
/** Success! */
#define RINF_SUCCESS                                    0
/** Warning, Output was truncated. */
#define RWRN_TRUNCATED                               (100)
/** Invalid bitmask combination of flags. */
#define RERR_INVALID_FLAGS                           (-99)
/** No data available. */
#define RERR_NO_DATA                                (-100)
/** Out of memory. */
#define RERR_NO_MEMORY                              (-101)
/** Invalid parameter to function. */
#define RERR_INVALID_PARAMETER                      (-102)
/** Too much data to fill in buffer. */
#define RERR_BUFFER_OVERFLOW                        (-103)
/** Duplicate operator. */
#define RERR_DUPLICATE_OPERATOR                     (-104)
/** Conflicting operator. */
#define RERR_CONFLICTING_OPERATORS                  (-105)
/** Invalid operator. */
#define RERR_INVALID_OPERATOR                       (-106)
/** Functor invalid. */
#define RERR_INVALID_FUNCTOR                        (-107)
/** Duplicate functor. */
#define RERR_DUPLICATE_FUNCTOR                      (-108)
/** Syntax error. */
#define RERR_SYNTAX_ERROR                           (-108)
/** Invalid RPN, parsing had failed. */
#define RERR_INVALID_RPN                            (-110)
/** Invalid expression for grammar. */
#define RERR_EXPRESSION_INVALID                     (-111)
/** Variable name is too long. */
#define RERR_VARIABLE_NAME_TOO_LONG                 (-112)
/** Variable name contains invalid characters. */
#define RERR_VARIABLE_NAME_INVALID                  (-113)
/** Variable undefined but trying to evaluate. */
#define RERR_VARIABLE_UNDEFINED                     (-114)
/** Variable cannot be reassigned. */
#define RERR_VARIABLE_CANNOT_REASSIGN               (-115)
/** Parenthesis unbalaned. */
#define RERR_PARENTHESIS_UNBALANCED                 (-116)
/** Some fundamental operator missing. */
#define RERR_BASIC_OPERATOR_MISSING                 (-117)
/** Invalid parameter separator position. */
#define RERR_PARANTHESIS_SEPARATOR_UNEXPECTED       (-118)
/** Too many parameters to functor. */
#define RERR_TOO_MANY_PARAMETERS                    (-119)
/** Too few parameters to functor. */
#define RERR_TOO_FEW_PARAMETERS                     (-120)
/** Invalid l-value assignment. */
#define RERR_INVALID_ASSIGNMENT                     (-121)
/** Circular variable dependency. */
#define RERR_CIRCULAR_DEPENDENCY                    (-122)
/** Cannot assign variable while evaluating a command. */
#define RERR_CANT_ASSIGN_VARIABLE_FOR_COMMAND       (-123)
/** Command failed. */
#define RERR_COMMAND_FAILED                         (-124)
/** Invalid parameter to a command. */
#define RERR_INVALID_COMMAND_PARAMETER              (-125)
/** Operator on unitialized object. */
#define RERR_NOT_INITIALIZED                        (-301)
/** Magic mismatch. */
#define RERR_BAD_MAGIC                              (-302)
/** Operation not supported. */
#define RERR_NOT_SUPPORTED                          (-303)
/** Not implemented. */
#define RERR_NOT_IMPLEMENTED                        (-304)
/** Undefined error. */
#define RERR_UNDEFINED                              (-666)
/** General failure, who is he? */
#define RERR_GENERAL_FAILURE                        (-667)
/** Evaluation will invoke Undefined Behaviour. */
#define RERR_UNDEFINED_BEHAVIOUR                    (-668)
/* ERR_SED_END */

#define RC_SUCCESS(rc)                              ( (int)(rc) >= RINF_SUCCESS )
#define RC_FAILURE(rc)                              ( !RC_SUCCESS(rc) )

/**
 * RCSTATUSMSG: Status Message
 */
typedef struct RCSTATUSMSG
{
    /** Symbolic name of error as a string. */
    const char *pszName;
    /** The error code. */
    int         rc;
} RCSTATUSMSG;
/** Pointer to a Status Message object. */
typedef RCSTATUSMSG *PRCSTATUSMSG;
/** Pointer to a const Status Message object. */
typedef const RCSTATUSMSG *PCRCSTATUSMSG;

extern const RCSTATUSMSG g_aStatusMsgs[];
extern const RCSTATUSMSG g_UnknownMsg;

PCRCSTATUSMSG StatusMsgForRC(int rc);

#endif /* ERRORS_H___ */

