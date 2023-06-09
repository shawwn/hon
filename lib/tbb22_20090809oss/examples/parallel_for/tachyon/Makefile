# Copyright 2005-2009 Intel Corporation.  All Rights Reserved.
#
# This file is part of Threading Building Blocks.
#
# Threading Building Blocks is free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public License
# version 2 as published by the Free Software Foundation.
#
# Threading Building Blocks is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Threading Building Blocks; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# As a special exception, you may use this file as part of a free software
# library without restriction.  Specifically, if other files instantiate
# templates or use macros or inline functions from this file, or you compile
# this file and link it with other files to produce an executable, this
# file does not by itself cause the resulting executable to be covered by
# the GNU General Public License.  This exception does not however
# invalidate any other reasons why the executable file might be covered by
# the GNU General Public License.

# The original source for this example is
# Copyright (c) 1994-2008 John E. Stone
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

# Common Makefile that builds and runs example.

# The C++ compiler
CXX = cl.exe

# Uncomment one of next lines to choose user interface type (console, gdiplus, direct draw)
#UI = con
UI = gdi
#UI = dd

# Machine architecture, auto-detected from TBB_ARCH_PLATFORM by default
# Use XARCH variable to change it. See index.html for more information
ARCH0 = $(TBB_ARCH_PLATFORM)-
ARCHA = $(ARCH0:\vc7.1-=)
ARCHB = $(ARCHA:\vc8-=)
ARCHC = $(ARCHB:\vc9-=)
ARCH3 = $(ARCHC:ia32=x86)
ARCH4 = $(ARCH3:intel64=AMD64)
XARCH = $(ARCH4:-=x86)

# The default dataset
DATASET = balls
# The default runtime arguments
ARGS =

# Add these for tbb/tbb1d release builds
CXXFLAGS_TBB_NDEBUG = $(CXXFLAGS)
LIBS_TBB_NDEBUG = tbb.lib $(LIBS)

# Add these for tbb/tbb1d debug builds
CXXFLAGS_TBB_DEBUG = $(CXXFLAGS) /D TBB_USE_DEBUG
LIBS_TBB_DEBUG =  tbb_debug.lib $(LIBS)


MAKEINC = ../../common/gui/Makefile.win
SOURCE = src/pthread.cpp src/video.cpp src/api.cpp src/apigeom.cpp src/apitrigeom.cpp src/bndbox.cpp src/box.cpp src/camera.cpp src/coordsys.cpp src/cylinder.cpp src/extvol.cpp src/getargs.cpp src/global.cpp src/grid.cpp src/imageio.cpp src/imap.cpp src/intersect.cpp src/jpeg.cpp src/light.cpp src/objbound.cpp src/parse.cpp src/plane.cpp src/ppm.cpp src/quadric.cpp src/render.cpp src/ring.cpp src/shade.cpp src/sphere.cpp src/texture.cpp src/tgafile.cpp src/trace_rest.cpp src/triangle.cpp src/ui.cpp src/util.cpp src/vector.cpp src/vol.cpp

# Targets
all: build run
release: build
debug: build_debug
test: run

build: build_serial build_tbb1d build_tbb
build_debug: build_serial_debug build_tbb1d_debug build_tbb_debug
run: run_serial run_tbb1d run_tbb

serial: build_serial run_serial
serial_debug: build_serial_debug run_serial
tbb: build_tbb run_tbb
tbb_debug: build_tbb_debug run_tbb
tbb1d: build_tbb1d run_tbb1d
tbb1d_debug: build_tbb1d_debug run_tbb1d

build_serial:
	@$(MAKE) -f $(MAKEINC) SOURCE="src/trace.serial.cpp $(SOURCE)" EXE=tachyon.serial.exe RCNAME=gui UI=$(UI) CXX="$(CXX)" CXXFLAGS=$(CXXFLAGS) XARCH=$(XARCH) build_one
build_serial_debug:
	@$(MAKE) -f $(MAKEINC) SOURCE="src/trace.serial.cpp $(SOURCE)" EXE=tachyon.serial.exe RCNAME=gui UI=$(UI) DEBUG=_debug CXX="$(CXX)" CXXFLAGS=$(CXXFLAGS) XARCH=$(XARCH) build_one
run_serial:
	-.\tachyon.serial.exe $(ARGS) dat\$(DATASET).dat

build_tbb:
	@$(MAKE) -f $(MAKEINC) SOURCE="src/trace.tbb.cpp $(SOURCE)" EXE=tachyon.tbb.exe RCNAME=gui UI=$(UI) CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS_TBB_NDEBUG)" LFLAGS="$(LIBS_TBB_NDEBUG)" XARCH=$(XARCH) build_one
build_tbb_debug:
	@$(MAKE) -f $(MAKEINC) SOURCE="src/trace.tbb.cpp $(SOURCE)" EXE=tachyon.tbb.exe RCNAME=gui UI=$(UI) DEBUG=_debug CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS_TBB_DEBUG)" LFLAGS="$(LIBS_TBB_DEBUG)" XARCH=$(XARCH) build_one
run_tbb:
	-.\tachyon.tbb.exe $(ARGS) dat\$(DATASET).dat

build_tbb1d:
	@$(MAKE) -f $(MAKEINC) SOURCE="src/trace.tbb1d.cpp $(SOURCE)" EXE=tachyon.tbb1d.exe RCNAME=gui UI=$(UI) CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS_TBB_NDEBUG)" LFLAGS="$(LIBS_TBB_NDEBUG)" XARCH=$(XARCH) build_one
build_tbb1d_debug:
	@$(MAKE) -f $(MAKEINC) SOURCE="src/trace.tbb1d.cpp $(SOURCE)" EXE=tachyon.tbb1d.exe RCNAME=gui UI=$(UI) DEBUG=_debug CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS_TBB_DEBUG)" LFLAGS="$(LIBS_TBB_DEBUG)" XARCH=$(XARCH) build_one
run_tbb1d:
	-.\tachyon.tbb1d.exe $(ARGS) dat\$(DATASET).dat


clean:
	@cmd.exe /C del tachyon.* *.manifest *.obj vc7.1\gui.res *.?db
