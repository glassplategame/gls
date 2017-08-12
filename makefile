#  Copyright (C) 2015  "Frostsnow" (Wade T. Cline).
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
CC = gcc
CFLAGS = -Wall -Werror --pedantic-errors -rdynamic --std=c99
LIBS = -lbsd -lpthread

client_files = board cargs flub global gls log client plate
client_objs=${client_files:=.o}
server_files = board flub global gls log plate player server
server_objs=${server_files:=.o}
files=board client flub global gls log plate player server
objs=${files:=.o}

# Default rule: compile only the client.
default: client

# Remove all generated output.
clean: tidy
	rm -f gls
	rm -f glsd

# Compile the client.
client: ${client_objs}
	${CC} -o gls ${LIBS} ${client_objs}

${objs}: %.o: %.c %.h
	${CC} ${CFLAGS} -c $<

# Remove extraneous output.
tidy:
	rm -f ${client_objs}
	rm -f ${server_objs}

# Compile the server.
server: ${server_objs}
	${CC} -o glsd ${LIBS} ${server_objs}
