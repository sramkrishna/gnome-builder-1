/* ide-autotools-pipeline-addin.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "ide-autotools-pipeline-addin"

#include "ide-autotools-autogen-stage.h"
#include "ide-autotools-build-system.h"
#include "ide-autotools-makecache-stage.h"
#include "ide-autotools-pipeline-addin.h"

static gboolean
register_autoreconf_stage (IdeAutotoolsPipelineAddin  *self,
                           IdeBuildPipeline           *pipeline,
                           GError                    **error)
{
  g_autofree gchar *configure_path = NULL;
  g_autoptr(IdeBuildStage) stage = NULL;
  IdeContext *context;
  const gchar *srcdir;
  gboolean completed;
  guint stage_id;

  g_assert (IDE_IS_AUTOTOOLS_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));

  context = ide_object_get_context (IDE_OBJECT (self));
  configure_path = ide_build_pipeline_build_srcdir_path (pipeline, "configure", NULL);
  completed = g_file_test (configure_path, G_FILE_TEST_IS_REGULAR);
  srcdir = ide_build_pipeline_get_srcdir (pipeline);

  stage = g_object_new (IDE_TYPE_AUTOTOOLS_AUTOGEN_STAGE,
                        "completed", completed,
                        "context", context,
                        "srcdir", srcdir,
                        NULL);

  stage_id = ide_build_pipeline_connect (pipeline, IDE_BUILD_PHASE_AUTOGEN, 0, stage);

  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static gint
compare_mtime (const gchar *path_a,
               const gchar *path_b)
{
  g_autoptr(GFile) file_a = g_file_new_for_path (path_a);
  g_autoptr(GFile) file_b = g_file_new_for_path (path_b);
  g_autoptr(GFileInfo) info_a = NULL;
  g_autoptr(GFileInfo) info_b = NULL;
  gint64 ret = 0;

  info_a = g_file_query_info (file_a,
                              G_FILE_ATTRIBUTE_TIME_MODIFIED,
                              G_FILE_QUERY_INFO_NONE,
                              NULL,
                              NULL);

  info_b = g_file_query_info (file_b,
                              G_FILE_ATTRIBUTE_TIME_MODIFIED,
                              G_FILE_QUERY_INFO_NONE,
                              NULL,
                              NULL);

  ret = (gint64)g_file_info_get_attribute_uint64 (info_a, G_FILE_ATTRIBUTE_TIME_MODIFIED) -
        (gint64)g_file_info_get_attribute_uint64 (info_b, G_FILE_ATTRIBUTE_TIME_MODIFIED);

  if (ret < 0)
    return -1;
  else if (ret > 0)
    return 1;
  return 0;
}

static void
check_configure_status (IdeAutotoolsPipelineAddin *self,
                        IdeBuildPipeline          *pipeline,
                        GCancellable              *cancellable,
                        IdeBuildStage             *stage)
{
  g_autofree gchar *configure_ac = NULL;
  g_autofree gchar *configure = NULL;
  g_autofree gchar *config_status = NULL;
  g_autofree gchar *makefile = NULL;

  IDE_ENTRY;

  g_assert (IDE_IS_AUTOTOOLS_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_BUILD_STAGE (stage));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  configure = ide_build_pipeline_build_srcdir_path (pipeline, "configure", NULL);
  configure_ac = ide_build_pipeline_build_srcdir_path (pipeline, "configure.ac", NULL);
  config_status = ide_build_pipeline_build_builddir_path (pipeline, "config.status", NULL);
  makefile = ide_build_pipeline_build_builddir_path (pipeline, "Makefile", NULL);

  IDE_TRACE_MSG (" configure.ac is at %s", configure_ac);
  IDE_TRACE_MSG (" configure is at %s", configure);
  IDE_TRACE_MSG (" config.status is at %s", config_status);
  IDE_TRACE_MSG (" makefile is at %s", makefile);

  /*
   * First make sure some essential files exist. If not, we need to run the
   * configure process.
   *
   * TODO: This may take some tweaking if we ever try to reuse existing builds
   *       that were performed in-tree.
   */
  if (!g_file_test (configure_ac, G_FILE_TEST_IS_REGULAR) ||
      !g_file_test (configure, G_FILE_TEST_IS_REGULAR) ||
      !g_file_test (config_status, G_FILE_TEST_IS_REGULAR) ||
      !g_file_test (makefile, G_FILE_TEST_IS_REGULAR))
    {
      ide_build_stage_set_completed (stage, FALSE);
      IDE_EXIT;
    }

  /*
   * Now make sure that config.status and Makefile are indeed newer than
   * our configure script.
   */
  if (compare_mtime (configure_ac, configure) < 0 &&
      compare_mtime (configure, config_status) < 0 &&
      compare_mtime (configure, makefile) < 0)
    {
      /*
       * TODO: It would be fancy if we could look at '^ac_cs_config=' to determine
       * if the configure args match what we expect. But this is a bit more
       * complicated than simply a string comparison.
       */
      ide_build_stage_set_completed (stage, TRUE);
      IDE_EXIT;
    }

  ide_build_stage_set_completed (stage, FALSE);

  IDE_EXIT;
}

static gboolean
register_configure_stage (IdeAutotoolsPipelineAddin  *self,
                          IdeBuildPipeline           *pipeline,
                          GError                    **error)
{
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;
  g_autoptr(IdeBuildStage) stage = NULL;
  IdeConfiguration *configuration;
  IdeDevice *device;
  g_autofree gchar *configure_path = NULL;
  g_autofree gchar *host_arg = NULL;
  const gchar *system_type;
  const gchar *config_opts;
  const gchar *prefix;
  guint stage_id;

  g_assert (IDE_IS_AUTOTOOLS_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));

  launcher = ide_build_pipeline_create_launcher (pipeline, error);

  if (launcher == NULL)
    return FALSE;

  configure_path = ide_build_pipeline_build_srcdir_path (pipeline, "configure", NULL);
  ide_subprocess_launcher_push_argv (launcher, configure_path);

  /* --host=triplet */
  configuration = ide_build_pipeline_get_configuration (pipeline);
  device = ide_configuration_get_device (configuration);
  system_type = ide_device_get_system_type (device);
  host_arg = g_strdup_printf ("--host=%s", system_type);
  ide_subprocess_launcher_push_argv (launcher, host_arg);

  /*
   * Parse the configure options as defined in the build configuration and append
   * them to configure.
   */

  config_opts = ide_configuration_get_config_opts (configuration);
  prefix = ide_configuration_get_prefix (configuration);

  if (prefix != NULL)
    {
      g_autofree gchar *prefix_arg = g_strdup_printf ("--prefix=%s", prefix);
      ide_subprocess_launcher_push_argv (launcher, prefix_arg);
    }

  if (!ide_str_empty0 (config_opts))
    {
      g_auto(GStrv) argv = NULL;
      gint argc = 0;

      if (!g_shell_parse_argv (config_opts, &argc, &argv, error))
        return FALSE;

      for (gint i = 0; i < argc; i++)
        ide_subprocess_launcher_push_argv (launcher, argv[i]);
    }

  stage = g_object_new (IDE_TYPE_BUILD_STAGE_LAUNCHER,
                        "context", ide_object_get_context (IDE_OBJECT (self)),
                        "launcher", launcher,
                        NULL);

  /*
   * If the Makefile exists within the builddir, we will assume the
   * project has been initially configured correctly. Otherwise, every
   * time the user opens the project they have to go through a full
   * re-configure and build.
   *
   * Should the user need to perform an autogen, a manual rebuild is
   * easily achieved so this seems to be the sensible default.
   *
   * If we were to do this "correctly", we would look at config.status to
   * match the "ac_cs_config" variable to what we set. However, that is
   * influenced by environment variables, so its a bit non-trivial.
   */
  g_signal_connect_object (stage,
                           "query",
                           G_CALLBACK (check_configure_status),
                           self,
                           G_CONNECT_SWAPPED);

  stage_id = ide_build_pipeline_connect (pipeline, IDE_BUILD_PHASE_CONFIGURE, 0, stage);

  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static void
make_stage_query (IdeAutotoolsPipelineAddin *self,
                  IdeBuildPipeline          *pipeline,
                  GCancellable              *cancellable,
                  IdeBuildStageLauncher     *stage)
{
  g_assert (IDE_IS_AUTOTOOLS_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));
  g_assert (IDE_IS_BUILD_STAGE_LAUNCHER (stage));

  /* Always rely on make to determine up-to-date status */
  ide_build_stage_set_completed (IDE_BUILD_STAGE (stage), FALSE);
}

G_GNUC_NULL_TERMINATED static gboolean
register_make_stage (IdeAutotoolsPipelineAddin  *self,
                     IdeBuildPipeline           *pipeline,
                     IdeRuntime                 *runtime,
                     const gchar                *log_file,
                     gboolean                    ignore_exit_code,
                     gboolean                    include_clean,
                     IdeBuildPhase               phase,
                     GError                    **error,
                     const gchar                *make,
                     const gchar                *first_target,
                     ...)
{
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;
  g_autoptr(IdeBuildStage) stage = NULL;
  IdeConfiguration *configuration;
  g_autofree gchar *j = NULL;
  IdeContext *context;
  guint stage_id;
  gint parallel;
  va_list args;

  g_assert (IDE_IS_AUTOTOOLS_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_RUNTIME (runtime));

  context = ide_object_get_context (IDE_OBJECT (pipeline));
  configuration = ide_build_pipeline_get_configuration (pipeline);

  launcher = ide_build_pipeline_create_launcher (pipeline, error);

  if (launcher == NULL)
    return FALSE;

  parallel = ide_configuration_get_parallelism (configuration);

  if (parallel == -1)
    j = g_strdup_printf ("-j%u", g_get_num_processors () + 1);
  else if (parallel == 0)
    j = g_strdup_printf ("-j%u", g_get_num_processors ());
  else
    j = g_strdup_printf ("-j%u", parallel);

  ide_subprocess_launcher_push_argv (launcher, make);
  ide_subprocess_launcher_push_argv (launcher, j);

  /* We want silent rules when possible */
  ide_subprocess_launcher_push_argv (launcher, "V=0");

  va_start (args, first_target);
  do
    ide_subprocess_launcher_push_argv (launcher, first_target);
  while (NULL != (first_target = va_arg (args, const gchar *)));
  va_end (args);

  stage = ide_build_stage_launcher_new (context, launcher);

  if (log_file != NULL)
    ide_build_stage_set_stdout_path (stage, log_file);

  if (ignore_exit_code)
    ide_build_stage_launcher_set_ignore_exit_status (IDE_BUILD_STAGE_LAUNCHER (stage), TRUE);

  g_signal_connect_object (stage,
                           "query",
                           G_CALLBACK (make_stage_query),
                           self,
                           G_CONNECT_SWAPPED);

  if (include_clean)
    {
      g_autoptr(IdeSubprocessLauncher) clean_launcher = NULL;

      clean_launcher = ide_build_pipeline_create_launcher (pipeline, error);

      if (clean_launcher == NULL)
        return FALSE;

      ide_subprocess_launcher_push_argv (clean_launcher, make);
      ide_subprocess_launcher_push_argv (clean_launcher, "clean");

      ide_build_stage_launcher_set_clean_launcher (IDE_BUILD_STAGE_LAUNCHER (stage), clean_launcher);
    }

  stage_id = ide_build_pipeline_connect (pipeline, phase, 0, stage);

  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static gboolean
register_makecache_stage (IdeAutotoolsPipelineAddin  *self,
                          IdeBuildPipeline           *pipeline,
                          GError                    **error)
{
  g_autoptr(IdeBuildStage) stage = NULL;
  guint stage_id;

  g_assert (IDE_IS_AUTOTOOLS_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));

  if (NULL == (stage = ide_autotools_makecache_stage_new_for_pipeline (pipeline, error)))
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_CONFIGURE | IDE_BUILD_PHASE_AFTER,
                                         0,
                                         stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);

  return TRUE;
}

static void
ide_autotools_pipeline_addin_load (IdeBuildPipelineAddin *addin,
                                   IdeBuildPipeline      *pipeline)
{
  IdeAutotoolsPipelineAddin *self = (IdeAutotoolsPipelineAddin *)addin;
  g_autoptr(GError) error = NULL;
  IdeConfiguration *config;
  IdeBuildSystem *build_system;
  IdeContext *context;
  IdeRuntime *runtime;
  const gchar *make = "make";

  g_assert (IDE_IS_AUTOTOOLS_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));

  context = ide_object_get_context (IDE_OBJECT (addin));
  build_system = ide_context_get_build_system (context);
  config = ide_build_pipeline_get_configuration (pipeline);
  runtime = ide_configuration_get_runtime (config);

  if (!IDE_IS_AUTOTOOLS_BUILD_SYSTEM (build_system))
    return;

  if (ide_runtime_contains_program_in_path (runtime, "gmake", NULL))
    make = "gmake";

  if (!register_autoreconf_stage (self, pipeline, &error) ||
      !register_configure_stage (self, pipeline, &error) ||
      !register_makecache_stage (self, pipeline, &error) ||
      !register_make_stage (self, pipeline, runtime, NULL, FALSE, TRUE, IDE_BUILD_PHASE_BUILD, &error, make, "all", NULL) ||
      !register_make_stage (self, pipeline, runtime, NULL, FALSE, FALSE, IDE_BUILD_PHASE_INSTALL, &error, make, "install", NULL))
    {
      g_assert (error != NULL);
      g_warning ("Failed to create autotools launcher: %s", error->message);
      return;
    }
}

/* GObject Boilerplate */

static void
addin_iface_init (IdeBuildPipelineAddinInterface *iface)
{
  iface->load = ide_autotools_pipeline_addin_load;
}

struct _IdeAutotoolsPipelineAddin { IdeObject parent; };

G_DEFINE_TYPE_WITH_CODE (IdeAutotoolsPipelineAddin, ide_autotools_pipeline_addin, IDE_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_BUILD_PIPELINE_ADDIN, addin_iface_init))

static void
ide_autotools_pipeline_addin_class_init (IdeAutotoolsPipelineAddinClass *klass)
{
}

static void
ide_autotools_pipeline_addin_init (IdeAutotoolsPipelineAddin *self)
{
}
