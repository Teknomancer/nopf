/* $Id: Colors.h 168 2011-10-12 17:24:32Z marshan $ */
/** @file
 * xTerm color definitions.
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

#ifndef COLORS_H___
#define COLORS_H___

/** @todo I quickly need to make this runtime toggle-able */

# define TCOLOR_RESET           "\033[0m"
# define TCOLOR_BLACK           "\033[0;30m"
# define TCOLOR_RED             "\033[0;31m"
# define TCOLOR_GREEN           "\033[0;32m"
# define TCOLOR_YELLOW          "\033[0;33m"
# define TCOLOR_BLUE            "\033[0;34m"
# define TCOLOR_PURPLE          "\033[0;35m"
# define TCOLOR_CYAN            "\033[0;36m"
# define TCOLOR_WHITE           "\033[0;37m"
# define TCOLOR_BOLD_BLACK      "\033[1;30m"
# define TCOLOR_BOLD_RED        "\033[1;31m"
# define TCOLOR_BOLD_GREEN      "\033[1;32m"
# define TCOLOR_BOLD_YELLOW     "\033[1;33m"
# define TCOLOR_BOLD_BLUE       "\033[1;34m"
# define TCOLOR_BOLD_PURPLE     "\033[1;35m"
# define TCOLOR_BOLD_CYAN       "\033[1;36m"
# define TCOLOR_BOLD_WHITE      "\033[1;37m"

/*
txtred='\033[0;31m' # Red
txtgrn='\033[0;32m' # Green
txtylw='\033[0;33m' # Yellow
txtblu='\033[0;34m' # Blue
txtpur='\033[0;35m' # Purple
txtcyn='\033[0;36m' # Cyan
txtwht='\033[0;37m' # White
bldblk='\033[1;30m' # Black - Bold
bldred='\033[1;31m' # Red
bldgrn='\033[1;32m' # Green
bldylw='\033[1;33m' # Yellow
bldblu='\033[1;34m' # Blue
bldpur='\033[1;35m' # Purple
bldcyn='\033[1;36m' # Cyan
bldwht='\033[1;37m' # White
unkblk='\033[4;30m' # Black - Underline
undred='\033[4;31m' # Red
undgrn='\033[4;32m' # Green
undylw='\033[4;33m' # Yellow
undblu='\033[4;34m' # Blue
undpur='\033[4;35m' # Purple
undcyn='\033[4;36m' # Cyan
undwht='\033[4;37m' # White
bakblk='\033[40m'   # Black - Background
bakred='\033[41m'   # Red
badgrn='\033[42m'   # Green
bakylw='\033[43m'   # Yellow
bakblu='\033[44m'   # Blue
bakpur='\033[45m'   # Purple
bakcyn='\033[46m'   # Cyan
bakwht='\033[47m'   # White
txtrst='\033[0m'    # Text Reset

*/

#endif /* COLORS_H___ */

