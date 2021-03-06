//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
/*
 * djudge, an online judge solution.
 * Copyright (C) 2009 Jakob Truelsen
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
#ifndef __results_hh__
#define __results_hh__
#include <string>

#define STR(x)   #x
#define XSTR(x)  STR(x)

#define RUN_SUCCESS                0
#define RUN_EXIT_NOT_ZERO          1
#define RUN_PRESENTATION_ERROR     2
#define RUN_TIME_LIMIT_EXCEEDED    3
#define RUN_ABNORMAL_TERMINATION   4
#define RUN_RUNTIME_ERROR          5
#define RUN_OUTPUT_LIMIT_EXCEEDED  6
#define RUN_INTERNAL_ERROR         7
#define RUN_COMPILATION_TIME_LIMIT_EXCEEDED 8
#define RUN_COMPILATION_ERROR      9
#define RUN_INVALID_ENTRY          10
#define RUN_BAD_COMMAND            11
#define RUN_PENDING                12
#define RUN_EXTRACT_ERROR          13
#define RUN_WRONG_OUTPUT           14
#define RUN_INVALID_MEMORY_REFERENCE 15
#define RUN_MEMORY_LIMIT_EXCEEDED  16
#define RUN_DEVIDE_BY_ZERO        17
std::string resultMessage(int res);

#endif //__results_hh__
