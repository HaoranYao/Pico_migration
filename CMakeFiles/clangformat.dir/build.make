# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/haoran/quic/picoquic

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/haoran/quic/picoquic

# Utility rule file for clangformat.

# Include the progress variables for this target.
include CMakeFiles/clangformat.dir/progress.make

CMakeFiles/clangformat:
	clang-format -style=Webkit -i /home/haoran/quic/picoquic/CMakeFiles/3.10.2/CompilerIdC/CMakeCCompilerId.c /home/haoran/quic/picoquic/CMakeFiles/3.16.4/CompilerIdC/CMakeCCompilerId.c /home/haoran/quic/picoquic/CMakeFiles/feature_tests.c /home/haoran/quic/picoquic/loglib/cidset.c /home/haoran/quic/picoquic/loglib/csv.c /home/haoran/quic/picoquic/loglib/logconvert.c /home/haoran/quic/picoquic/loglib/logreader.c /home/haoran/quic/picoquic/loglib/qlog.c /home/haoran/quic/picoquic/loglib/svg.c /home/haoran/quic/picoquic/picohttp/democlient.c /home/haoran/quic/picoquic/picohttp/demoserver.c /home/haoran/quic/picoquic/picohttp/h3zero.c /home/haoran/quic/picoquic/picohttp/siduck.c /home/haoran/quic/picoquic/picohttp_t/picohttp_t.c /home/haoran/quic/picoquic/picolog/picolog.c /home/haoran/quic/picoquic/picoqinq/qinqclient.c /home/haoran/quic/picoquic/picoqinq/qinqproto.c /home/haoran/quic/picoquic/picoqinq/qinqserver.c /home/haoran/quic/picoquic/picoqinq_t/picoqinq_t.c /home/haoran/quic/picoquic/picoquic/bbr.c /home/haoran/quic/picoquic/picoquic/bytestream.c /home/haoran/quic/picoquic/picoquic/cc_common.c /home/haoran/quic/picoquic/picoquic/cubic.c /home/haoran/quic/picoquic/picoquic/fastcc.c /home/haoran/quic/picoquic/picoquic/frames.c /home/haoran/quic/picoquic/picoquic/intformat.c /home/haoran/quic/picoquic/picoquic/logger.c /home/haoran/quic/picoquic/picoquic/logwriter.c /home/haoran/quic/picoquic/picoquic/newreno.c /home/haoran/quic/picoquic/picoquic/packet.c /home/haoran/quic/picoquic/picoquic/picohash.c /home/haoran/quic/picoquic/picoquic/picosocks.c /home/haoran/quic/picoquic/picoquic/picosplay.c /home/haoran/quic/picoquic/picoquic/quicctx.c /home/haoran/quic/picoquic/picoquic/sacks.c /home/haoran/quic/picoquic/picoquic/sender.c /home/haoran/quic/picoquic/picoquic/sim_link.c /home/haoran/quic/picoquic/picoquic/spinbit.c /home/haoran/quic/picoquic/picoquic/ticket_store.c /home/haoran/quic/picoquic/picoquic/tls_api.c /home/haoran/quic/picoquic/picoquic/token_store.c /home/haoran/quic/picoquic/picoquic/transport.c /home/haoran/quic/picoquic/picoquic/util.c /home/haoran/quic/picoquic/picoquic_c2csv_t/picoquic_c2csv_t.c /home/haoran/quic/picoquic/picoquic_t/picoquic_t.c /home/haoran/quic/picoquic/picoquicfirst/getopt.c /home/haoran/quic/picoquic/picoquicfirst/picoquicdemo.c /home/haoran/quic/picoquic/picoquictest/ack_of_ack_test.c /home/haoran/quic/picoquic/picoquictest/bytestream_test.c /home/haoran/quic/picoquic/picoquictest/cleartext_aead_test.c /home/haoran/quic/picoquic/picoquictest/cnx_creation_test.c /home/haoran/quic/picoquic/picoquictest/h3zerotest.c /home/haoran/quic/picoquic/picoquictest/hashtest.c /home/haoran/quic/picoquic/picoquictest/intformattest.c /home/haoran/quic/picoquic/picoquictest/parseheadertest.c /home/haoran/quic/picoquic/picoquictest/pn2pn64test.c /home/haoran/quic/picoquic/picoquictest/qinqtest.c /home/haoran/quic/picoquic/picoquictest/sacktest.c /home/haoran/quic/picoquic/picoquictest/skip_frame_test.c /home/haoran/quic/picoquic/picoquictest/socket_test.c /home/haoran/quic/picoquic/picoquictest/splay_test.c /home/haoran/quic/picoquic/picoquictest/stream0_frame_test.c /home/haoran/quic/picoquic/picoquictest/stresstest.c /home/haoran/quic/picoquic/picoquictest/ticket_store_test.c /home/haoran/quic/picoquic/picoquictest/tls_api_test.c /home/haoran/quic/picoquic/picoquictest/transport_param_test.c /home/haoran/quic/picoquic/picoquictest/util_test.c /home/haoran/quic/picoquic/quicwind/quicwind_proc.c /home/haoran/quic/picoquic/UnitTest1/targetver.h /home/haoran/quic/picoquic/loglib/cidset.h /home/haoran/quic/picoquic/loglib/csv.h /home/haoran/quic/picoquic/loglib/logconvert.h /home/haoran/quic/picoquic/loglib/logreader.h /home/haoran/quic/picoquic/loglib/qlog.h /home/haoran/quic/picoquic/loglib/svg.h /home/haoran/quic/picoquic/picohttp/democlient.h /home/haoran/quic/picoquic/picohttp/demoserver.h /home/haoran/quic/picoquic/picohttp/h3zero.h /home/haoran/quic/picoquic/picohttp/siduck.h /home/haoran/quic/picoquic/picoqinq/qinqclient.h /home/haoran/quic/picoquic/picoqinq/qinqproto.h /home/haoran/quic/picoquic/picoqinq/qinqserver.h /home/haoran/quic/picoquic/picoquic/bytestream.h /home/haoran/quic/picoquic/picoquic/cc_common.h /home/haoran/quic/picoquic/picoquic/logwriter.h /home/haoran/quic/picoquic/picoquic/picohash.h /home/haoran/quic/picoquic/picoquic/picoquic.h /home/haoran/quic/picoquic/picoquic/picoquic_internal.h /home/haoran/quic/picoquic/picoquic/picoquic_logger.h /home/haoran/quic/picoquic/picoquic/picoquic_utils.h /home/haoran/quic/picoquic/picoquic/picosocks.h /home/haoran/quic/picoquic/picoquic/picosplay.h /home/haoran/quic/picoquic/picoquic/picotlsapi.h /home/haoran/quic/picoquic/picoquic/tls_api.h /home/haoran/quic/picoquic/picoquic/wincompat.h /home/haoran/quic/picoquic/picoquicfirst/getopt.h /home/haoran/quic/picoquic/picoquictest/picoquictest.h /home/haoran/quic/picoquic/picoquictest/picoquictest_internal.h /home/haoran/quic/picoquic/quicwind/Resource.h /home/haoran/quic/picoquic/quicwind/quicwind.h /home/haoran/quic/picoquic/quicwind/targetver.h

clangformat: CMakeFiles/clangformat
clangformat: CMakeFiles/clangformat.dir/build.make

.PHONY : clangformat

# Rule to build all files generated by this target.
CMakeFiles/clangformat.dir/build: clangformat

.PHONY : CMakeFiles/clangformat.dir/build

CMakeFiles/clangformat.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/clangformat.dir/cmake_clean.cmake
.PHONY : CMakeFiles/clangformat.dir/clean

CMakeFiles/clangformat.dir/depend:
	cd /home/haoran/quic/picoquic && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/haoran/quic/picoquic /home/haoran/quic/picoquic /home/haoran/quic/picoquic /home/haoran/quic/picoquic /home/haoran/quic/picoquic/CMakeFiles/clangformat.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/clangformat.dir/depend
