# 
# Copyright (C) 2011 Ramshankar (aka Teknomancer)
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 

# Scan for text within the markers
/ERR_SED_BEGIN/,/ERR_SED_END/ {

# if (#define) goto grokdefs
/^[[:space:]]*#[[:space:]]*define/b grokdefs

}

# We don't need the rest of the file
d
b end

##
# Scan the defines and spit out symbolic name followed by value in the form "{ "SYMBOLIC"\nVALUE }," (outer quotes not included)
# @todo \n doesn't work on darwin for some damn reason, can't figure it out now, revisit.
:grokdefs
s/^[[:space:]]*#[[:space:]]*define[[:space:]]*\([[:alnum:]_]*\)[[:space:]]*\(.*\)[[:space:]]*$/    { "\1", \1 },/
b end

# skip to next
:end

