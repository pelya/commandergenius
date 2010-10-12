------------------------------------------------------------------------
-- Copyright (C) 2006 Ronald Lamprecht
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; either version 2
-- of the License, or (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License along
-- with this program; if not, write to the Free Software Foundation, Inc.,
-- 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
--
------------------------------------------------------------------------

-- basic security
dofile = function () error ("security error: dofile not supported", 2) end

-- module security
module = function () error ("security error: module not supported", 2) end
require = function () error ("security error: require not supported", 2) end
package.cpath = function () error ("security error: package.cpath not supported", 2) end
package.loadlib = function () error ("security error: package.loadlib not supported", 2) end
package.path = function () error ("security error: package.path not supported", 2) end
package.preload = function () error ("security error: package.preload not supported", 2) end
package.seeall = function () error ("security error: package.seeall not supported", 2) end

-- io security
io.close = function () error ("security error: io.close not supported", 2) end
io.flush = function () error ("security error: io.flush not supported", 2) end
io.input = function () error ("security error: io.input not supported", 2) end
io.lines = function () error ("security error: io.lines not supported", 2) end
io.open = function () error ("security error: io.open not supported", 2) end
io.output = function () error ("security error: io.output not supported", 2) end
io.popen = function () error ("security error: io.popen not supported", 2) end
io.read = function () error ("security error: io.read not supported", 2) end
io.tmpfile = function () error ("security error: io.tmpfile not supported", 2) end
io.type = function () error ("security error: io.type not supported", 2) end
io.write = function () error ("security error: io.write not supported", 2) end
-- io 4.0 compatibility security
readfrom = function () error ("security error: readfrom not supported", 2) end
writeto = function () error ("security error: writeto not supported", 2) end
appendto = function () error ("security error: appendto not supported", 2) end
flush = function () error ("security error: flush not supported", 2) end
seek = function () error ("security error: seek not supported", 2) end
read = function () error ("security error: read not supported", 2) end
write = function () error ("security error: write not supported", 2) end

-- os security
os.clock = function () error ("security error: os.clock not supported", 2) end
os.date = function () error ("security error: os.date not supported", 2) end
os.difftime = function () error ("security error: os.difftime not supported", 2) end
os.time = function () error ("security error: os.time not supported", 2) end
os.execute = function () error ("security error: os.execute not supported", 2) end
os.exit = function () error ("security error: os.exit not supported", 2) end
os.getenv = function () error ("security error: os.getenv not supported", 2) end
os.remove = function () error ("security error: os.remove not supported", 2) end
os.rename = function () error ("security error: os.rename not supported", 2) end
os.setlocale = function () error ("security error: os.setlocale not supported", 2) end
os.tmpname = function () error ("security error: os.tmpname not supported", 2) end

