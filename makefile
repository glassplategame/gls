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
CC=gcc
CFLAGS=-Wall -Werror

# Compile the client.
client:
	gcc -c board.c
	gcc -c client.c
	gcc -o gls board.o client.o

# Remove all generated output.
clean:
	rm -f gls
	rm -f glsd

# Remove extraneous output.
tidy:
	rm *.o

# Compile the server.
server:
	gcc -c board.c
	gcc -c log.c
	gcc -c server.c
	gcc -o glsd board.o log.o server.o
