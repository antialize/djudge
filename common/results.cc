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
#include "results.hh"

std::string resultsMessage(int res) {
	switch(res) {
	case RUN_SUCCESS:
		return "Successfull";
	case RUN_EXIT_NOT_ZERO:
		return "The program terminated with a none zero exit code";
	case RUN_PRESENTATION_ERROR:
		return "There was a presentation error in the output";
	case RUN_TIME_LIMIT_EXCEEDED:
		return "The time limit was exceeded";
	case RUN_ABNORMAL_TERMINATION:
		return "The program terminated abnormaly";
	case RUN_RUNTIME_ERROR:
		return "The program faild to run";
	case RUN_OUTPUT_LIMIT_EXCEEDED:
		return "Tho profram exceeded the output limit";
	case RUN_INTERNAL_ERROR:
		return "Internal error,  this should not happen UPS";
	case RUN_COMPILATION_TIME_LIMIT_EXCEEDED:
		return "The progarm to too long to compile";
	case RUN_COMPILATION_ERROR:
		return "Error during compilation";
	case RUN_INVALID_ENTRY:
		return "The entry did not exist, or was malformed";
	default:
		return "BAD RESULT CODE";
  }
}
