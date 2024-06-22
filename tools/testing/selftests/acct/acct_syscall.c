// SPDX-License-Identifier: GPL-2.0

/* kselftest for acct() system call
 *  The acct() system call enables or disables process accounting.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#include "../kselftest.h"

int main(void)
{
	// Setting up kselftest framework
	ksft_print_header();
	ksft_set_plan(1);

	// Create file to log closed processes
	char filename[] = "process_log";
	FILE *fp;

	fp = fopen(filename, "w");

	int i = acct(filename);

	// Handle error conditions
	if (i) {
		switch (errno) {
		case EPERM:
			ksft_test_result_error("%s. Please run the test as root.\n",
				strerror(errno));
			break;

		case EACCES:
			ksft_test_result_error("Insufficient privilege.\n");
			break;

		case EIO:
			ksft_test_result_error("Error writing to the file: %s.\n", filename);
			break;

		default:
			ksft_test_result_error("%s.\n", strerror(errno));
			break;
		}

		remove(filename);
		fclose(fp);
		ksft_finished();
		return 1;
	}

	// Create child process and wait for it to terminate.
	pid_t child_pid;

	child_pid = fork();

	if (child_pid < 0) {
		ksft_test_result_error("Process failed\n");
		ksft_finished();
		return 1;
	} else if (child_pid == 0) {
		ksft_print_msg("Child process successfully created!\n");
	} else {
		wait(NULL);
		fseek(fp, 0L, SEEK_END);
		int sz = ftell(fp);

		ksft_print_msg("Parent process successfully created!\n");

		i = acct(NULL);

		if (sz <= 0) {
			ksft_test_result_fail("Terminated child process not logged");
			ksft_exit_fail();
			return 1;
		}

		ksft_test_result_pass("Successfully logged terminated process.\n");
		remove(filename);
		fclose(fp);
		ksft_exit_pass();
		return 0;
	}

	return 1;
}
