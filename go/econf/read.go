package econf

import (
	"errors"
	"os"
	"path/filepath"
	"strings"
)

const (
	defaultRunSubdir = "/run"
	defaultEtcSubdir = "/etc"
)

// readFileWithCallback mirrors getfilecontents.c's read_file_with_callback,
// minus the ownership/permission/symlink checks (see go/econf doc.go).
func readFileWithCallback(f *File, fileName, delim, comment string, callback FileCallback) error {
	if _, err := os.Lstat(fileName); err != nil {
		return ErrNoFile
	}
	if callback != nil && !callback(fileName) {
		return ErrParsingCallbackFailed
	}
	return f.readFile(fileName, delim, comment)
}

// ReadFile parses fileName and returns its contents.
func ReadFile(fileName, delim, comment string) (*File, error) {
	return ReadFileWithCallback(fileName, delim, comment, nil)
}

// ReadFileWithCallback parses fileName and returns its contents. callback is
// invoked with the path before parsing; returning false aborts the read
// with ErrParsingCallbackFailed.
func ReadFileWithCallback(fileName, delim, comment string, callback FileCallback) (*File, error) {
	f := &File{}
	if err := readFileWithCallback(f, fileName, delim, comment, callback); err != nil {
		return nil, err
	}
	return f, nil
}

// checkConfDir mirrors mergefiles.c's check_conf_dir: it scans dirPath for
// files ending in configSuffix and parses each one. A missing or unreadable
// directory is not an error (mirroring scandir's "num_dirs <= 0" case).
func checkConfDir(dirPath, configSuffix, delim, comment string, joinSameEntries, pythonStyle bool, callback FileCallback) ([]*File, error) {
	dirEntries, err := os.ReadDir(dirPath)
	if err != nil {
		return nil, nil
	}

	var files []*File
	for _, de := range dirEntries {
		name := de.Name()
		if len(configSuffix) >= len(name) || !strings.HasSuffix(name, configSuffix) {
			continue
		}
		f := &File{joinSameEntries: joinSameEntries, pythonStyle: pythonStyle}
		if err := readFileWithCallback(f, filepath.Join(dirPath, name), delim, comment, callback); err != nil {
			return files, err
		}
		files = append(files, f)
	}
	return files, nil
}

// readConfigHistory mirrors readconfig.c's readConfigHistoryWithCallback:
// it finds the single highest-priority "main" configuration file (if
// config_name is non-empty) and then every drop-in file across all
// parseDirs, returning them in the order they should be merged (lowest to
// highest priority).
func readConfigHistory(parseDirs []string, configName, configSuffix, delim, comment string, joinSameEntries, pythonStyle bool, confDirs []string, callback FileCallback) ([]*File, error) {
	suffix := ""
	var files []*File

	if configName != "" {
		if configSuffix != "" {
			if strings.HasPrefix(configSuffix, ".") {
				suffix = configSuffix
			} else {
				suffix = "." + configSuffix
			}
		}

		for i := len(parseDirs); i > 0; i-- {
			filename := parseDirs[i-1] + "/" + configName + suffix
			candidate := &File{joinSameEntries: joinSameEntries, pythonStyle: pythonStyle}
			err := readFileWithCallback(candidate, filename, delim, comment, callback)
			if err != nil {
				if errors.Is(err, ErrNoFile) {
					continue
				}
				return nil, err
			}
			files = append(files, candidate)
			break
		}
	}

	configureDirs := confDirs
	if len(configureDirs) == 0 {
		configureDirs = []string{suffix + ".d"}
	}

	for _, parseDir := range parseDirs {
		projectPath := parseDir + "/" + configName
		for _, confDirSuffix := range configureDirs {
			dropins, err := checkConfDir(projectPath+confDirSuffix, suffix, delim, comment, joinSameEntries, pythonStyle, callback)
			if err != nil {
				return nil, err
			}
			files = append(files, dropins...)
		}
	}

	if len(files) == 0 {
		return nil, ErrNoFile
	}
	return files, nil
}

// mergeFileList mirrors mergefiles.c's merge_econf_files: files are merged
// in order, except that a file is skipped entirely if a later (higher
// priority) file in the list shares its basename.
func mergeFileList(files []*File) (*File, error) {
	if len(files) == 0 {
		return nil, ErrError
	}
	merged := files[0]
	for i := 1; i < len(files); i++ {
		current := files[i]
		currentBase := filepath.Base(current.path)

		superseded := false
		for j := i + 1; j < len(files); j++ {
			if filepath.Base(files[j].path) == currentBase {
				superseded = true
				break
			}
		}
		if superseded {
			continue
		}

		var err error
		merged, err = MergeFiles(merged, current)
		if err != nil {
			return nil, err
		}
	}
	return merged, nil
}

// ReadConfig evaluates key/values of a configuration by reading and merging
// all matching files from the vendor, runtime and admin directories. project,
// usrSubdir, configName and configSuffix may be "" to mean "not set" (see
// econf_readConfig in include/libeconf.h for the exact directory/precedence
// rules).
func ReadConfig(project, usrSubdir, configName, configSuffix, delim, comment string) (*File, error) {
	return ReadConfigWithCallback(project, usrSubdir, configName, configSuffix, delim, comment, nil)
}

// ReadConfigWithCallback behaves like ReadConfig, additionally invoking
// callback for every candidate file before it is parsed.
func ReadConfigWithCallback(project, usrSubdir, configName, configSuffix, delim, comment string, callback FileCallback) (*File, error) {
	f := &File{}
	if err := f.ReadConfigWithCallback(project, usrSubdir, configName, configSuffix, delim, comment, callback); err != nil {
		return nil, err
	}
	return f, nil
}

// ReadConfig behaves like the package-level ReadConfig, except it runs
// against f itself, mirroring econf_readConfig's reuse of an
// already-configured econf_file: f's JoinSameEntries, PythonStyle,
// PARSING_DIRS and CONFIG_DIRS settings (set via NewKeyFileWithOptions)
// are honored instead of the package-level defaults. f's prior contents,
// including those same options, are discarded and replaced by the result.
func (f *File) ReadConfig(project, usrSubdir, configName, configSuffix, delim, comment string) error {
	return f.ReadConfigWithCallback(project, usrSubdir, configName, configSuffix, delim, comment, nil)
}

// ReadConfigWithCallback behaves like (*File).ReadConfig, additionally
// invoking callback for every candidate file before it is parsed.
func (f *File) ReadConfigWithCallback(project, usrSubdir, configName, configSuffix, delim, comment string, callback FileCallback) error {
	confDirs := f.confDirs
	if configName == "" {
		configName = project
		project = ""
		if len(confDirs) == 0 {
			confDirs = []string{".d"}
		}
	}

	parseDirs := f.parseDirs
	if len(parseDirs) == 0 {
		usrDir := usrSubdir
		runDir := defaultRunSubdir
		etcDir := defaultEtcSubdir
		if project != "" {
			usrDir = usrSubdir + "/" + project
			runDir = defaultRunSubdir + "/" + project
			etcDir = defaultEtcSubdir + "/" + project
		}
		parseDirs = []string{usrDir, runDir, etcDir}
	}

	files, err := readConfigHistory(parseDirs, configName, configSuffix, delim, comment, f.joinSameEntries, f.pythonStyle, confDirs, callback)
	if err != nil {
		return err
	}
	merged, err := mergeFileList(files)
	if err != nil {
		return err
	}
	*f = *merged
	return nil
}

// ReadDirsHistory evaluates key/values for every configuration file found in
// usrConfDir and etcConfDir and returns one *File per file found, in the
// order they were read (lowest to highest precedence).
func ReadDirsHistory(usrConfDir, etcConfDir, configName, configSuffix, delim, comment string) ([]*File, error) {
	return ReadDirsHistoryWithCallback(usrConfDir, etcConfDir, configName, configSuffix, delim, comment, nil)
}

// ReadDirsHistoryWithCallback behaves like ReadDirsHistory, additionally
// invoking callback for every candidate file before it is parsed.
func ReadDirsHistoryWithCallback(usrConfDir, etcConfDir, configName, configSuffix, delim, comment string, callback FileCallback) ([]*File, error) {
	parseDirs := []string{usrConfDir, etcConfDir}
	return readConfigHistory(parseDirs, configName, configSuffix, delim, comment, false, false, nil, callback)
}
