# Documentation

## `static class EConf`
### Public Methods

```cs
public static EConfErr ReadFile(out EConfFile result, string fileName, string delim, string comment);
```
> Implements `econf_readFile`

Process the file of the given `fileName` and save its contents into the result object.
Param       | Description
---         | --- 
`result`    | Content of parsed file.
`fileName`  | Absolute path of parsed file.
`delim`     | Delimiters of key/value e.g. "\t =". If delim contains space characters AND none space characters, multiline values are not parseable.
`comment`   | Array of characters which define the start of a comment.

Returns `EConfErr.Success` or error code.

---
```cs
public static EConfErr MergeFiles(out EConfFile mergedFile, EConfFile usrFile, EConfFile etcFile)
```
> Implements `econf_mergeFiles`

Merge the contents of two key_files objects. Entries in `etcFile` will be preferred.
Comment and delimiter tag will be taken from `usrFile`. This can be changed
by calling the functions `EConfFile.CommentTag` and `EConfFile.DelimiterTag`.

Param           | Description
---             | --- 
`mergedFile`    | Merged data.
`usrFile`       | First data block which has to be merged.
`etcFile`       | Second data block which has to be merged.

Returns `EConfErr.Success` or error code.

---
```cs
public static EConfErr ReadDirs(out EConfFile keyFile, string usrConfDir, string etcConfDir, string projectName, string? configSuffix, string delim, string comment)
```
> Implements `econf_readDirs`

Evaluating key/values of a given configuration by reading and merging all
needed/available files in two different directories (normally in /usr/etc and /etc).

Param           | Description
---             | --- 
`keyFile`       | Content of the parsed file(s).
`usrConfDir`    | Absolute path of the first directory (normally "/usr/etc").
`etcConfDir`    | Absolute path of the second directory (normally "/etc").
`projectName`   | Base name of the configuration file.
`configSuffix`  | Suffix of the configuration file. Can also be null.
`delim`         | Delimiters of key/value e.g. "\t =" If delim contains space characters AND none space characters, multiline values are not parseable.
`comment`       | String which defines the start of a comment.

Returns `EConfErr.Success` or error code.

---
```cs
public static EConfErr ReadDirsHistory(out EConfFile[] keyFiles, string userConfDir, string etcConfDir, string projectName, string? configSuffix, string delim, string comment)
```
> Implements `econf_readDirsHistory`

Evaluating key/values for every given configuration files in two different
directories (normally in /usr/etc and /etc). Returns a list of read configuration
files and their values.

Param           | Description
---             | --- 
`keyFiles`      | Array of parsed file(s). Each entry includes all key/value, path, comments,... information of the regarding file.
`usrConfDir`    | Absolute path of the first directory (normally "/usr/etc").
`etcConfDir`    | Absolute path of the second directory (normally "/etc").
`projectName`   | Base name of the configuration file.
`configSuffix`  | Suffix of the configuration file. Can also be null.
`delim`         | Delimiters of key/value e.g. "\t =" If delim contains space characters AND none space characters, multiline values are not parseable.
`comment`       | String which defines the start of a comment.

Returns `EConfErr.Success` or error code.

---
```cs
public static EConfErr SetConfDirs(string[] dirPostFixList)
```
> Implements `econf_set_conf_dirs`

Set a list of directory structures (with order) which describes
the directories in which the files have to be parsed.

Param               | Description
---                 | --- 
`dirPostFixList`    | List of directory structures. 

> For example, with the given list: `{"/conf.d/", ".d/", "/"}` files in following directories will be parsed: 
> ```
> "<default_dirs>/<project_name>.<suffix>.d/" 
> "<default_dirs>/<project_name>/conf.d/" 
> "<default_dirs>/<project_name>.d/" 
> "<default_dirs>/<project_name>/"
> ```
> The entry `"<default_dirs>/<project_name>.<suffix>.d/"` will be added automatically.

Returns `EConfErr.Success` or error code.

---
```cs
public static ulong ErrLocation(string path)
```
> Implements `econf_errLocation`

Info about where the error has happened.

Param   | Description
---     | --- 
`path`  | Path of the last scanned file.

Returns `EConfErr.Success` or error code.

---
```cs
public Dictionary<string, string> ToDictionary(string? group);
```
Converts all KeyValue pairs of a group to a dictionary.

Param   | Description
---     | --- 
`group`  | The group to focus on. An empty string for anonymous groups, or null for all groups.

Returns the selected KeyValue pairs as a Dictionary.

---
```cs
public void AppendDictionary(string? group, Dictionary<string, string> dict)
```
Appends all KeyValue pairs of `dict` to the file.

Param   | Description
---     | --- 
`group` | The group to append to, or an empty string/null for anonymous group.
`dict`  | The data to append.

## `class EConfFile`
### Constructors
```cs
public EConfFile();
```
> Implements `econf_newIniFile`

Creates a new empty `.ini` file instance. The delimiter will be `=` and comments will start with `#`.

---
```cs
public EConfFile(char delimiter, char comment);
```
> Implements `econf_newKeyFile`

Creates a new empty file instance with the given delimiter and comment character.
Param       | Description
---         | --- 
`delimiter` | Delimiter of key/value e.g. "=".
`comment`   | Character which defines the start of a comment.

### Properties
---
```cs
public char? CommentTag { get; }
```
> Implements `econf_getCommentTag` and `econf_setCommentTag`

Gets or sets the comment character for this file.

---
```cs
public char DelimiterTag { get; set; }
```
> Implements `econf_getDelimiterTag` and `econf_setDelimiterTag`

Gets or sets the delimiter character for this file.

---
```cs
public string? Path { get; }
```
> Implements `econf_getPath`

Gets the path of this file.

### Public Methods
---
```cs
public int      GetValueInt(string? group, string key);
public uint     GetValueUInt(string? group, string key);
public long     GetValueInt64(string? group, string key);
public ulong    GetValueUInt64(string? group, string key);
public float    GetValueFloat(string? group, string key);
public double   GetValueDouble(string? group, string key);
public string   GetValueString(string? group, string key);
public bool     GetValueBool(string? group, string key);
```
> Implements `econf_getValue<type>`

Gets the value for the specified key in the specified group.

Param   | Description
---     | --- 
`group` | The group of the key, an empty string or null for an anonymous group. 
`key`   | The name of the key. May not be null.

Returns the value of the specified key.

---
```cs
public EConfErr TryGetValue(string? group, string key, out int result);
public EConfErr TryGetValue(string? group, string key, out uint result);
public EConfErr TryGetValue(string? group, string key, out long result);
public EConfErr TryGetValue(string? group, string key, out ulong result);
public EConfErr TryGetValue(string? group, string key, out float result);
public EConfErr TryGetValue(string? group, string key, out double result);
public EConfErr TryGetValue(string? group, string key, out string result);
public EConfErr TryGetValue(string? group, string key, out bool result);
```
Attempts to get a value from the specified key and attempts to store it in `result`.

Param       | Description
---         | --- 
`group`     | The group of the key, an empty string or null for an anonymous group. 
`key`       | The name of the key. May not be null.
`result`    | If successful, the read value. Otherwise the default value (For `string` this is `string.Empty`).

Returns `EConfErr.Success` or error code.

---
```cs
public EConfErr SetValue(string? group, string key, int value);
public EConfErr SetValue(string? group, string key, uint value);
public EConfErr SetValue(string? group, string key, long value);
public EConfErr SetValue(string? group, string key, ulong value);
public EConfErr SetValue(string? group, string key, float value);
public EConfErr SetValue(string? group, string key, double value);
public EConfErr SetValue(string? group, string key, string value);
public EConfErr SetValue(string? group, string key, bool value);
```
> Implements `econf_setValue<type>`

Sets a value for the specified key.

Param       | Description
---         | --- 
`group`     | The group of the key, an empty string or null for an anonymous group. 
`key`       | The name of the key. May not be null.
`value`     | The value to set.

Returns `EConfErr.Success` or error code.

---
```cs
public string[] GetGroups();
```
> Implements `econf_getGroups`

Gets all groups in this file.

Returns the names of all groups in this file.

---
```cs
public string[] GetKeys(string? group);
```
> Implements `econf_getKeys`

Gets all groups in this file.

Returns the names of all groups in this file.

### Interface Implementations
```cs
public void IDisposable.Dispose();
```
> Implements `econf_freeFile`

Releases all resources associated with this file.
