NAME=expertsystem
CC = gcc
LINK_FLAGS = -Wall -Wextra -lm -ldl
CC_FLAGS = -c
INCS = -Iinclude
BUILDDIR=build/
SRCS = ${wildcard src/*.c}
OBJS= ${SRCS:.c=.o}
OBJS_TARGET=${addprefix ${BUILDDIR},${subst /,_,${OBJS}}}

# Style constants
RED=\033[0;31m
GREEN=\033[0;32m
YELLOW=\033[0;33m
BLUE=\033[0;34m
PURPLE=\033[0;35m
CYAN=\033[0;36m
NC=\033[0m # No Color

all : ${NAME}
	@echo "${GREEN}✔️  Done building..${NC}"

debug : BUILDDIR=build_dbg/
debug : OBJS_TARGET=${addprefix ${BUILDDIR},${subst /,_,${OBJS}}}
debug : CC_FLAGS += -D __DEBUG__
debug : LINK_FLAGS += -fsanitize=address -g3
debug : ${NAME}
	@echo "${GREEN}✔️  Done building debug..${NC}"

${NAME}: ${OBJS_TARGET}
	@echo "${GREEN}😏  Linking..${NC}"
	@${CC} ${BUILDDIR}*.o ${LINK_FLAGS} -o ${NAME}

build_dbg/%.o : ${OBJS}
	@echo "${GREEN}📇  Compile debug finish..${NC}"

build/%.o : ${OBJS}
	@echo "${GREEN}📇  Compile finish..${NC}"

.c.o :
	@echo "${GREEN}📇  Compiling $<..${NC}"
	@${CC} ${INCS} ${CC_FLAGS} $< -o ${BUILDDIR}${subst /,_,$@}

clean : 
	@echo "${YELLOW}🗑️  Removing Objects..${NC}"
	@rm -rf ${BUILDDIR}*.o


clean_dbg : BUILDDIR = build_dbg/
clean_dbg : clean

fclean : clean
	@echo "${YELLOW}🗑️  Removing ${NAME}..${NC}"
	@rm -rf ${NAME}

fclean_dbg : clean_dbg
	@echo "${YELLOW}🗑️  Removing ${NAME}..${NC}"
	@rm -rf ${NAME}

re : fclean all

.PHONY : re clean fclean all