/**
 * \file main.c
 * \author Ray Ruan(a1173522112@163.com)
 * \date 2021-10-13
 */

#include <stdio.h>
#include <glib.h>

/* flags parse by command argument */
static gchar *ppid = (gchar *)DEFAULT_PPID;
static gchar *logfile = (gchar *)DEFAULT_LOG_FILE;
static gchar *uuid;

static void set_log_path(const char *);
static void option_parse(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	option_parse(argc, argv);

	set_log_path(logfile);

	g_message("init");
	g_message("ppid: %s", ppid);
	g_message("uuid: %s", uuid);

	return 0;
}

/**
 * \brief Global message output callback.
 *
 * The log output format is "MESSAGE 211012 19:11:27 main.cc:42 > message".
 *
 * \param user_data Pointer of FILE where can output.
 */
static GLogWriterOutput log_write(
		GLogLevelFlags log_level,
		const GLogField *fields,
		gsize n_fields,
		gpointer user_data)
{
	FILE *logfile = (FILE *)user_data;
	GString *logstr = g_string_new("");

	/* print message type */
	switch (log_level) {
	case G_LOG_LEVEL_DEBUG: g_string_append_printf(logstr, "DEBUG "); break;
	case G_LOG_LEVEL_MESSAGE: g_string_append_printf(logstr, "MESSAGE "); break;
	case G_LOG_LEVEL_WARNING: g_string_append_printf(logstr, "WARNING "); break;
	case G_LOG_LEVEL_ERROR: g_string_append_printf(logstr, "ERROR "); break;
	default: g_string_append_printf(logstr, "UNKNOW:%d ", log_level); break;
	}

	/* get system time */
	time_t ttime = time(NULL);
	char s_time[64];
	strftime(s_time, sizeof(s_time), "%y%m%d %T", localtime(&ttime));
	g_string_append_printf(logstr, "%s ", s_time);

	/* make filename sorter */
	const char *sort_filename = strrchr((const char *)fields[1].value, '/');
	sort_filename = sort_filename != NULL ? sort_filename+1 : (const char *)fields[1].value;
	/* format: MESSAGE 211012 19:11:27 main.cc:42 > message */
	g_string_append_printf(logstr, "%s:%s > %s\n",
			sort_filename, fields[2].value, fields[4].value);

	if (log_level == G_LOG_LEVEL_ERROR) {
		/* print in log file and terminal */
		g_print("%s", logstr->str);
		fprintf(logfile, "%s", logstr->str);
#ifndef NDEBUG
	} else if (log_level == G_LOG_LEVEL_DEBUG) {
		/* only print in terminal */
		g_print("%s", logstr->str);
#endif
	} else {
		/* print in log file */
		fprintf(logfile, "%s", logstr->str);
	}
	fflush(logfile);

	g_string_free(logstr, TRUE);
	return G_LOG_WRITER_HANDLED;
}

/**
 * \brief Open log file and set log writer.
 * \param log_path Log file name inluding path.
 */
static void set_log_path(const char *log_path)
{
	FILE *logfile = fopen(log_path, "a");
	if (logfile == NULL)
		g_error("open log file err: %s\n", strerror(errno));

	g_log_set_writer_func(log_write, (void *)logfile, NULL);
}

static GOptionEntry entries[] = {
	{"ppid", 'p', 0, G_OPTION_ARG_STRING, &ppid, "Product id in apple mfi website", DEFAULT_PPID},
	{"logfile", 'l', 0, G_OPTION_ARG_STRING, &logfile, "The path to log file", DEFAULT_LOG_FILE},
	{NULL}
};

/**
 * \brief Parse the command line parameters.
 */
static void option_parse(int argc, char *argv[])
{
	gboolean ret = FALSE;
	GOptionContext *context = g_option_context_new("device-uuid");

	/* setting parser */
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_set_summary(context, "This program use to enable and disable apple token");

	/* parse it */
	ret = g_option_context_parse(context, &argc, &argv, NULL);
	if (!ret)
		g_error("parse err");
	g_option_context_free(context);

	/* get uuid */
	if (argc < 2)
		g_error(g_option_context_get_help(context, TRUE, NULL));
	uuid = argv[1];
}
