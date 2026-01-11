#pragma once

/*
 * popen_noshell: A faster implementation of popen() and system() for Linux.
 * Copyright (c) 2009 Ivan Zahariev (famzah)
 * Version: 1.0
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; under version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses>.
 */

// _GNU_SOURCE must be defined as early as possible
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <spawn.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>

// because of C++, we can't call err() or errx() within the child, because they call exit(), 
// and _exit() is what must be called; so we wrap
#define _ERR(EVAL, FMT, ...) {warn(FMT, ##__VA_ARGS__); _exit(EVAL);}

struct vpopenStruct {
    FILE *fp = nullptr;
    pid_t pid = 0;
} pclose_arg;

extern char **environ;

namespace VPOPEN {

    int popen_noshell_reopen_fd_to_dev_null(int fd, posix_spawn_file_actions_t *file_actions) {
        if (posix_spawn_file_actions_addclose(file_actions, fd) != 0) return -1;
        if (posix_spawn_file_actions_addopen(file_actions, fd, "/dev/null", O_RDWR, 0600) < 0)
            return -1;
        return 0;
    }

    int _popen_noshell_close_and_dup(int pipefd[2], int closed_pipefd, int target_fd,
                                     posix_spawn_file_actions_t *file_actions) {
        int dupped_pipefd = (closed_pipefd == 0 ? 1 : 0); // get the FD of the other end of the pipe

        if (posix_spawn_file_actions_addclose(file_actions, pipefd[closed_pipefd]) != 0) return -1;
        if (posix_spawn_file_actions_addclose(file_actions, target_fd) != 0) return -1;
        if (posix_spawn_file_actions_adddup2(file_actions, pipefd[dupped_pipefd], target_fd) < 0)
            return -1;
        if (posix_spawn_file_actions_addclose(file_actions, pipefd[dupped_pipefd]) != 0) return -1;
        return 0;
    }

    // returns the new PID if called in POPEN_NOSHELL_MODE_POSIX_SPAWN
    // otherwise returns 0
    pid_t _popen_noshell_child_process(int pipefd_0, int pipefd_1, const char *file,
                                       const char *const *argv) {
        posix_spawn_file_actions_t file_actions_obj;
        if (posix_spawn_file_actions_init(&file_actions_obj) != 0) {
            _ERR(255, "posix_spawn_file_actions_init()");
        }

        int closed_child_fd = STDIN_FILENO;        /* re-open STDIN to /dev/null */
        int closed_pipe_fd = 0;            /* close read end of pipe */
        int dupped_child_fd = STDOUT_FILENO;    /* dup the other pipe end to STDOUT */
        int pipefd[2] = {pipefd_0, pipefd_1};

        if (popen_noshell_reopen_fd_to_dev_null(closed_child_fd, &file_actions_obj) != 0) {
            _ERR(255, "popen_noshell_reopen_fd_to_dev_null(%d)", closed_child_fd);
        }
        if (_popen_noshell_close_and_dup(pipefd, closed_pipe_fd, dupped_child_fd,
                                         &file_actions_obj) != 0) {
            _ERR(255, "_popen_noshell_close_and_dup(%d ,%d)", closed_pipe_fd, dupped_child_fd);
        }

        pid_t child_pid;
        if (posix_spawn(&child_pid, file, &file_actions_obj, nullptr, (char *const *) argv,
                        environ) < 0) {
            warn("posix_spawn(\"%s\") inside the child", file);
            if (posix_spawn_file_actions_destroy(&file_actions_obj) != 0) {
                warn("posix_spawn_file_actions_destroy()");
            }
            return 0;
        }
        if (posix_spawn_file_actions_destroy(&file_actions_obj) != 0) {
            warn("posix_spawn_file_actions_destroy()");
        }
        return child_pid;
    }

    FILE *popen_noshell(const char *file, const char *const *argv) {
        int pipefd[2]; // 0 -> READ, 1 -> WRITE ends
        if (pipe2(pipefd, O_CLOEXEC) != 0) return nullptr;

        pid_t pid = _popen_noshell_child_process(pipefd[0], pipefd[1], file, argv);
        if (pid == 0) return nullptr;
        if (close(pipefd[1/*write*/]) != 0) return nullptr;

        auto fp = fdopen(pipefd[0/*read*/], "r");
        if (fp) {
            pclose_arg.fp = fp;
            pclose_arg.pid = pid;
        }
        return fp;
    }

    int pclose_noshell() {
        int status = 0;
        if (fclose(pclose_arg.fp) != 0) return -1;
        if (waitpid(pclose_arg.pid, &status, __WALL) != pclose_arg.pid) return -2;
        return status;
    }

    void vpopen(const char *absPath, const char *argv[], char *buf, const size_t len) {
        auto fp = popen_noshell(absPath, (const char *const *) argv);
        if (!fp) {
            buf[0] = 0;
            fprintf(stderr, "%s() open 失败 [%d]:[%s]", __FUNCTION__, errno, strerror(errno));
            return;
        }

        auto resLen = fread(buf, 1, len, fp);
        if (resLen <= 0) {
            buf[0] = 0;
        } else if (buf[resLen - 1] == '\n') {
            buf[resLen - 1] = 0;
            resLen--;
        } else {
            buf[resLen] = 0;
        }

        // https://man7.org/linux/man-pages/man2/waitpid.2.html
        // wait 的 status参数 只用了低 16位
        // 高8位 记录进程调用exit退出的状态（正常退出）
        // 低8位 记录进程接受到的信号 （非正常退出）
        auto status = pclose_noshell();
        if (status < 0 || (status & 0xff))
            fprintf(stderr, "%s() close 异常status[%d] [%d]:[%s]", __FUNCTION__, status, errno,
                    strerror(errno));
    }
};
