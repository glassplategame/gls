/**
 *  Implementation of Dunbar's "Glass Plate Game" game, which is based off of
 *  Herman Hesse's novel, "The Glass Bead Game".
 *
 *  Argument definitions and parser for the client.
 *
 *  Copyright (C) 2017  Wade T. Cline.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef cargs_H
#define cargs_H

#include <getopt.h>

#include "gls.h"

// Client arguments.
struct cargs {
	// Nickname to use.
	char nick[GLS_NAME_LENGTH];
};

// Parse client arguments.
struct flub* cargs_parse(struct cargs* args, int argc, char* argv[]);

#endif // cargs_H
