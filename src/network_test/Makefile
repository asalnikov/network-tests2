#
# This file is a part of the PARUS project and  makes the core of the parus system
# Copyright (C) 2006  Alexey N. Salnikov
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Alexey N. Salnikov (salnikov@cmc.msu.ru)
#

#
# This file makes the nework_test of the parus system
#

include ../../config

MODULES_SUPPORT=TRUE

FILES_COMMON=\
	tests_common.o

FILES_TEST2=\
	all_to_all2.o\
	async_one_to_one2.o\
	one_to_one2.o\
	send_recv_and_recv_send2.o\
	network_test2.o\
	test_noise_common.o\
	test_noise2.o\
	test_noise_blocking2.o\
	bcast2.o\
	get2.o\
	put2.o\
	parse_arguments.o

FILES_MODULES=\
	one_to_one2.so\
	all_to_all2.so

FILES_NT=\
	network_test2.o\
	parse_arguments.o

FILES_PRINT_FROM_TO=\
	print_from_to.o

MODULES_FLAG=
ifeq ($(MODULES_SUPPORT), TRUE)
	MODULES_FLAG+=-DMODULES_SUPPORT -O0
endif

	
               
EXECS= print_from_to convert_to_netcdf network_test2

all: $(EXECS)

ifeq ($(MODULES_SUPPORT), TRUE)
network_test2: $(FILES_MODULES) $(FILES_COMMON) $(FILES_NT)
	$(MPI_CLINKER) $(MPI_CLINKER_FLAGS) $(MPI_LIB_PATH) $(NETCDF_INCLUDE) -L ../core -o network_test2 $(FILES_NT) $(FILES_COMMON) $(MPI_LIBS) -lparus_network  -lnetcdf -lm -ldl
else
network_test2: $(FILES_TEST2) $(FILES_COMMON)
	$(MPI_CLINKER) $(MPI_CLINKER_FLAGS) $(MPI_LIB_PATH) $(NETCDF_INCLUDE) -L ../core -o network_test2 $(FILES_COMMON) $(FILES_TEST2) $(MPI_LIBS) -lparus_network  -lnetcdf -lm
endif


print_from_to: $(FILES_PRINT_FROM_TO)
	$(CCLINKER) $(CCLINKER_FLAGS) $(MPI_LIB_PATH) $(NETCDF_INCLUDE) -L ../core -o print_from_to $(FILES_PRINT_FROM_TO) $(MPI_LIBS) -lnetcdf -lm

convert_to_netcdf: convert_to_netcdf.o $(FILES_COMMON) 
	$(MPI_CCLINKER) $(MPI_CCLINKER_FLAGS) $(MPI_LIB_PATH) -L ../core -o  convert_to_netcdf convert_to_netcdf.o ../core/string_id_converters.o  $(FILES_COMMON) $(MPI_LIBS) -lparus_network -lnetcdf -lm


clean:
	rm  -f ./*.o
	rm  -f ./*.so
	rm  -f $(EXECS)

ifeq ($(MODULES_SUPPORT), TRUE)
install: all
	cp -rf $(EXECS) $(INSTALL_DIR)/bin/
	mkdir $(INSTALL_DIR)/bin/modules
	cp -rf $(FILES_MODULES) $(INSTALL_DIR)/bin/modules	
else
install: all
	cp -rf $(EXECS) $(INSTALL_DIR)/bin/
endif

%.so: %.c
	$(MPI_cc) -c -fPIC $(MODULES_FLAG) $(MPI_cc_FLAGS) $(MPI_cc_INCLUDE) -I ../core -I ../.. $^ -o $(^:.c=.o)
	$(MPI_CCLINKER) -shared -fPIC $(MPI_CCLINKER_FLAGS) $(MPI_LIB_PATH) -L ../core $(^:.c=.o) -o $@  -lparus_network_pic

%.o: %.c
	$(MPI_cc) $(MODULES_FLAG) $(MPI_cc_FLAGS) $(MPI_cc_INCLUDE) -I ../core -I ../.. -c $^ -o $@ 
   
%.o: %.cpp
	$(MPI_CC) $(MODULES_FLAG) $(MPI_CC_FLAGS) $(MPI_CC_INCLUDE) -I ../core -I ../.. -c $^ -o $@

