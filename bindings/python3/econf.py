"""
Econf provides functionality for interacting with Key-Value config files, like getting and setting values for read config files.

For more information please have a look at the API
"""
import ctypes.util
from enum import Enum
from dataclasses import dataclass
from typing import *
from ctypes import *

LIBNAME = ctypes.util.find_library("econf")
LIBECONF = CDLL(LIBNAME)


@dataclass
class EconfFile:
    """
    Class which points to the Key Value storage object
    """

    _ptr: c_void_p

    def __del__(self):
        free_file(self)


class EconfError(Enum):
    SUCCESS = 0
    ERROR = 1
    NOMEM = 2
    NOFILE = 3
    NOGROUP = 4
    NOKEY = 5
    EMPTYKEY = 6
    WRITEERROR = 7
    PARSE_ERROR = 8
    MISSING_BRACKET = 9
    MISSING_DELIMITER = 10
    EMPTY_SECTION_NAME = 11
    TEXT_AFTER_SECTION = 12
    FILE_LIST_IS_NULL = 13
    WRONG_BOOLEAN_VALUE = 14
    KEY_HAS_NULL_VALUE = 15
    WRONG_OWNER = 16
    WRONG_GROUP = 17
    WRONG_FILE_PERMISSION = 18
    WRONG_DIR_PERMISSION = 19
    ERROR_FILE_IS_SYM_LINK = 20
    PARSING_CALLBACK_FAILED = 21


ECONF_EXCEPTION = {
    EconfError.ERROR: Exception,
    EconfError.NOMEM: MemoryError,
    EconfError.NOFILE: FileNotFoundError,
    EconfError.NOGROUP: KeyError,
    EconfError.NOKEY: KeyError,
    EconfError.EMPTYKEY: KeyError,
    EconfError.WRITEERROR: OSError,
    EconfError.PARSE_ERROR: Exception,
    EconfError.MISSING_BRACKET: SyntaxError,
    EconfError.MISSING_DELIMITER: SyntaxError,
    EconfError.EMPTY_SECTION_NAME: SyntaxError,
    EconfError.TEXT_AFTER_SECTION: SyntaxError,
    EconfError.FILE_LIST_IS_NULL: ValueError,
    EconfError.WRONG_BOOLEAN_VALUE: ValueError,
    EconfError.KEY_HAS_NULL_VALUE: ValueError,
    EconfError.WRONG_OWNER: PermissionError,
    EconfError.WRONG_GROUP: PermissionError,
    EconfError.WRONG_FILE_PERMISSION: PermissionError,
    EconfError.WRONG_DIR_PERMISSION: PermissionError,
    EconfError.ERROR_FILE_IS_SYM_LINK: PermissionError,
    EconfError.PARSING_CALLBACK_FAILED: Exception
}


def _encode_str(string: str | bytes) -> bytes:
    if isinstance(string, str):
        string = string.encode("utf-8")
    elif not isinstance(string, bytes):
        raise TypeError("Input must be a string or bytes")
    return string


def _ensure_valid_char(char: str | bytes) -> bytes:
    char = _encode_str(char)
    if len(char) > 1:
        raise ValueError("Only single characters are allowed as comment and delimiter")
    return char


def _ensure_valid_int(val: int) -> int:
    if isinstance(val, int):
        c_val = c_int64(val)
        if not c_val.value == val:
            raise ValueError(
                "Integer overflow found, only up to 64 bit signed integers are supported"
            )
        return c_val
    else:
        raise TypeError(f"parameter {val} is not an integer")


def _ensure_valid_uint(val: int) -> int:
    if isinstance(val, int) and (val >= 0):
        c_val = c_uint64(val)
        if not c_val.value == val:
            raise ValueError(
                "Integer overflow found, only up to 64 bit unsigned integers are supported"
            )
        return c_val
    else:
        raise TypeError(f"parameter {val} is not an unsigned integer")


def set_value(
    ef: EconfFile, group: str | bytes, key: str | bytes, value: int | float | str | bool
) -> None:
    """
    Dynamically set a value in a keyfile and returns a status code

    :param ef: EconfFile object to set value in
    :param group: group of the key to be changed
    :param key: key to be changed
    :param value: desired value
    :return: Nothing
    """
    if isinstance(value, int):
        if value >= 0:
            set_uint_value(ef, group, key, value)
        else:
            set_int_value(ef, group, key, value)
    elif isinstance(value, float):
        set_float_value(ef, group, key, value)
    elif isinstance(value, str) | isinstance(value, bytes):
        set_string_value(ef, group, key, value)
    elif isinstance(value, bool):
        set_bool_value(ef, group, key, value)
    else:
        raise TypeError(f"parameter {val} is not one of the supported types")


def read_file(
    file_name: str | bytes, delim: str | bytes, comment: str | bytes
) -> EconfFile:
    """
    Read a config file and write the key-value pairs into a keyfile object

    :param file_name: absolute path of file to be parsed
    :param delim: delimiter of a key/value e.g. '='
    :param comment: string that defines the start of a comment e.g. '#'
    :return: Key-Value storage object
    """
    result = EconfFile(c_void_p(None))
    file_name = _encode_str(file_name)
    delim = _ensure_valid_char(delim)
    comment = _ensure_valid_char(comment)
    err = LIBECONF.econf_readFile(byref(result._ptr), file_name, delim, comment)
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"read_file failed with error: {err_string(err)}")
    return result


def read_file_with_callback(
    file_name: str | bytes,
    delim: str | bytes,
    comment: str | bytes,
    callback: Callable[[any], bool],
    callback_data: any,
) -> EconfFile:
    """
    Read a config file and write the key-value pairs into a keyfile object

    A user defined function will be called in order e.g. to check the correct file permissions.
    If the function returns False the parsing will be aborted and an Exception will be raised

    :param file_name: absolute path of file to be parsed
    :param delim: delimiter of a key/value e.g. '='
    :param comment: string that defines the start of a comment e.g. '#'
    :param callback: User defined function which will be called and returns a boolean
    :param callback_data: argument to be give to the callback function
    :return: Key-Value storage object
    """
    result = EconfFile(c_void_p(None))
    file_name = _encode_str(file_name)
    delim = _ensure_valid_char(delim)
    comment = _ensure_valid_char(comment)

    def callback_proxy(fake_data: c_void_p) -> c_bool:
        return callback(callback_data)

    CBFUNC = CFUNCTYPE(c_bool, c_void_p)
    cb_func = CBFUNC(callback_proxy)

    err = LIBECONF.econf_readFileWithCallback(
        byref(result._ptr), file_name, delim, comment, cb_func, c_void_p(None)
    )
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](
            f"read_file_with_callback failed with error: {err_string(err)}"
        )
    return result


def merge_files(usr_file: EconfFile, etc_file: EconfFile) -> EconfFile:
    """
    Merge the content of 2 keyfile objects

    :param usr_file: first EconfFile object
    :param etc_file: second EconfFile object
    :return: merged EconfFile object
    """
    merged_file = EconfFile(c_void_p())
    err = LIBECONF.econf_mergeFiles(
        byref(merged_file._ptr),
        usr_file._ptr,
        etc_file._ptr,
    )
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"merge_files failed with error: {err_string(err)}")
    return merged_file


def read_dirs(
    usr_conf_dir: str | bytes,
    etc_conf_dir: str | bytes,
    config_name: str | bytes,
    config_suffix: str | bytes,
    delim: str | bytes,
    comment: str | bytes,
) -> EconfFile:
    """
    Evaluating key/values of a given configuration by reading and merging all needed/available
    files from different directories.

    :param usr_conf_dir: absolute path of the first directory to be searched
    :param etc_conf_dir: absolute path of the second directory to be searched
    :param config_name: basename of the configuration file
    :param config_suffix: suffix of the configuration file
    :param delim: delimiter of a key/value e.g. '='
    :param comment: string that defines the start of a comment e.g. '#'
    :return: merged EconfFile object
    """
    result = EconfFile(c_void_p())
    usr_conf_dir = _encode_str(usr_conf_dir)
    etc_conf_dir = _encode_str(etc_conf_dir)
    config_name = _encode_str(config_name)
    config_suffix = _encode_str(config_suffix)
    delim = _ensure_valid_char(delim)
    comment = _ensure_valid_char(comment)
    err = LIBECONF.econf_readDirs(
        byref(result._ptr),
        usr_conf_dir,
        etc_conf_dir,
        config_name,
        config_suffix,
        delim,
        comment,
    )
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"read_dirs failed with error: {err_string(err)}")
    return result


def read_dirs_with_callback(
    usr_conf_dir: str | bytes,
    etc_conf_dir: str | bytes,
    config_name: str | bytes,
    config_suffix: str | bytes,
    delim: str | bytes,
    comment: str | bytes,
    callback: Callable[[any], bool],
    callback_data: any,
) -> EconfFile:
    """
    Evaluating key/values of a given configuration by reading and merging all needed/available
    files from different directories.

    For every file a user defined function will be called in order e.g. to check the correct file permissions.
    If the function returns False the parsing will be aborted and an Exception will be raised

    :param usr_conf_dir: absolute path of the first directory to be searched
    :param etc_conf_dir: absolute path of the second directory to be searched
    :param config_name: basename of the configuration file
    :param config_suffix: suffix of the configuration file
    :param delim: delimiter of a key/value e.g. '='
    :param comment: string that defines the start of a comment e.g. '#'
    :param callback: User defined function which will be called for each file and returns a boolean
    :param callback_data: argument to be give to the callback function
    :return: merged EconfFile object
    """
    result = EconfFile(c_void_p())
    usr_conf_dir = _encode_str(usr_conf_dir)
    etc_conf_dir = _encode_str(etc_conf_dir)
    config_name = _encode_str(config_name)
    config_suffix = _encode_str(config_suffix)
    delim = _ensure_valid_char(delim)
    comment = _ensure_valid_char(comment)

    def callback_proxy(fake_data: c_void_p):
        return callback(callback_data)

    CBFUNC = CFUNCTYPE(c_bool, c_void_p)
    cb_func = CBFUNC(callback_proxy)

    err = LIBECONF.econf_readDirsWithCallback(
        byref(result._ptr),
        usr_conf_dir,
        etc_conf_dir,
        config_name,
        config_suffix,
        delim,
        comment,
        cb_func,
        c_void_p(None),
    )
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](
            f"read_dirs_with_callback failed with error: {err_string(err)}"
        )
    return result

def read_config(
    project: str | bytes,
    usr_conf_dir: str | bytes,
    config_name: str | bytes,
    config_suffix: str | bytes,
    delim: str | bytes,
    comment: str | bytes,
) -> EconfFile:
    """
    Evaluating key/values of a given configuration by reading and merging all needed/available
    files from different directories.

    This call fulfills all requirements, defined by the Linux Userspace API (UAPI) Group
    chapter "Configuration Files Specification".

    :param project: name of the project used as subdirectory
    :param usr_conf_dir: absolute path of the first directory to be searched
    :param config_name: basename of the configuration file
    :param config_suffix: suffix of the configuration file
    :param delim: delimiter of a key/value e.g. '='
    :param comment: string that defines the start of a comment e.g. '#'
    :return: merged EconfFile object
    """
    result = EconfFile(c_void_p())
    project = _encode_str(project)
    usr_conf_dir = _encode_str(usr_conf_dir)
    config_name = _encode_str(config_name)
    config_suffix = _encode_str(config_suffix)
    delim = _ensure_valid_char(delim)
    comment = _ensure_valid_char(comment)
    err = LIBECONF.econf_readConfig(
        byref(result._ptr),
        project,
        usr_conf_dir,
        config_name,
        config_suffix,
        delim,
        comment,
    )
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"read_config failed with error: {err_string(err)}")
    return result

def read_config_with_callback(
    project: str | bytes,
    usr_conf_dir: str | bytes,
    config_name: str | bytes,
    config_suffix: str | bytes,
    delim: str | bytes,
    comment: str | bytes,
    callback: Callable[[any], bool],
    callback_data: any,
) -> EconfFile:
    """
    Evaluating key/values of a given configuration by reading and merging all needed/available
    files from different directories.

    For every file a user defined function will be called in order e.g. to check the correct file permissions.
    If the function returns False the parsing will be aborted and an Exception will be raised

    This call fulfills all requirements, defined by the Linux Userspace API (UAPI) Group
    chapter "Configuration Files Specification".

    :param project: name of the project used as subdirectory
    :param usr_conf_dir: absolute path of the first directory to be searched
    :param config_name: basename of the configuration file
    :param config_suffix: suffix of the configuration file
    :param delim: delimiter of a key/value e.g. '='
    :param comment: string that defines the start of a comment e.g. '#'
    :param callback: User defined function which will be called for each file and returns a boolean
    :param callback_data: argument to be give to the callback function
    :return: merged EconfFile object
    """
    result = EconfFile(c_void_p())
    project = _encode_str(project)
    usr_conf_dir = _encode_str(usr_conf_dir)
    config_name = _encode_str(config_name)
    config_suffix = _encode_str(config_suffix)
    delim = _ensure_valid_char(delim)
    comment = _ensure_valid_char(comment)

    def callback_proxy(fake_data: c_void_p):
        return callback(callback_data)

    CBFUNC = CFUNCTYPE(c_bool, c_void_p)
    cb_func = CBFUNC(callback_proxy)

    err = LIBECONF.econf_readConfigWithCallback(
        byref(result._ptr),
        project,
        usr_conf_dir,
        config_name,
        config_suffix,
        delim,
        comment,
        cb_func,
        c_void_p(None),
    )
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](
            f"read_config_with_callback failed with error: {err_string(err)}"
        )
    return result

def read_dirs_history(
    usr_conf_dir: str | bytes,
    etc_conf_dir: str | bytes,
    config_name: str | bytes,
    config_suffix: str | bytes,
    delim: str | bytes,
    comment: str | bytes,
) -> list[EconfFile]:
    """

    Evaluating key/values of a given configuration by reading and merging all needed/available
    files from different directories.

    :param usr_conf_dir: absolute path of the first directory to be searched
    :param etc_conf_dir: absolute path of the second directory to be searched
    :param config_name: basename of the configuration file
    :param config_suffix: suffix of the configuration file
    :param delim: delimiter of a key/value e.g. '='
    :param comment: string that defines the start of a comment e.g. '#'
    :return: list of EconfFile objects
    """
    key_files = c_void_p(None)
    size = c_size_t()
    usr_conf_dir = _encode_str(usr_conf_dir)
    etc_conf_dir = _encode_str(etc_conf_dir)
    config_name = _encode_str(config_name)
    config_suffix = _encode_str(config_suffix)
    delim = _ensure_valid_char(delim)
    comment = _ensure_valid_char(comment)
    err = LIBECONF.econf_readDirsHistory(
        byref(key_files),
        byref(size),
        usr_conf_dir,
        etc_conf_dir,
        config_name,
        config_suffix,
        delim,
        comment,
    )
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"read_dirs_history failed with error: {err_string(err)}")

    arr = cast(key_files, POINTER(c_void_p * size.value))
    result = [EconfFile(c_void_p(i)) for i in arr.contents]
    return result


def read_dirs_history_with_callback(
    usr_conf_dir: str | bytes,
    etc_conf_dir: str | bytes,
    config_name: str | bytes,
    config_suffix: str | bytes,
    delim: str | bytes,
    comment: str | bytes,
    callback: Callable[[any], bool],
    callback_data: any,
) -> EconfFile:
    """
    Evaluating key/values of a given configuration by reading and merging all needed/available
    files from different directories.

    For every file a user defined function will be called in order e.g. to check the correct file permissions.
    If the function returns False the parsing will be aborted and an Exception will be raised

    :param usr_conf_dir: absolute path of the first directory to be searched
    :param etc_conf_dir: absolute path of the second directory to be searched
    :param config_name: basename of the configuration file
    :param config_suffix: suffix of the configuration file
    :param delim: delimiter of a key/value e.g. '='
    :param comment: string that defines the start of a comment e.g. '#'
    :param callback: User defined function which will be called for each file and returns a boolean
    :param callback_data: argument to be give to the callback function
    :return: list of EconfFile objects
    """
    key_files = c_void_p(None)
    size = c_size_t()
    usr_conf_dir = _encode_str(usr_conf_dir)
    etc_conf_dir = _encode_str(etc_conf_dir)
    config_name = _encode_str(config_name)
    config_suffix = _encode_str(config_suffix)
    delim = _ensure_valid_char(delim)
    comment = _ensure_valid_char(comment)

    def callback_proxy(fake_data: c_void_p):
        return callback(callback_data)

    CBFUNC = CFUNCTYPE(c_bool, c_void_p)
    cb_func = CBFUNC(callback_proxy)

    err = LIBECONF.econf_readDirsHistoryWithCallback(
        byref(key_files),
        byref(size),
        usr_conf_dir,
        etc_conf_dir,
        config_name,
        config_suffix,
        delim,
        comment,
        cb_func,
        c_void_p(None),
    )
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](
            f"read_dirs_history_with_callback failed with error: {err_string(err)}"
        )

    arr = cast(key_files, POINTER(c_void_p * size.value))
    result = [EconfFile(c_void_p(i)) for i in arr.contents]
    return result


def new_key_file(delim: str | bytes, comment: str | bytes) -> EconfFile:
    """
    Create a new empty keyfile

    :param delim: delimiter of a key/value e.g. '='
    :param comment: string that defines the start of a comment e.g. '#'
    :return: created EconfFile object
    """
    result = EconfFile(c_void_p())
    delim = c_char(_ensure_valid_char(delim))
    comment = c_char(_ensure_valid_char(comment))
    err = LIBECONF.econf_newKeyFile(byref(result._ptr), delim, comment)
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"new_key_file failed with error: {err_string(err)}")
    return result


def new_ini_file() -> EconfFile:
    """
    Create a new empty keyfile with delimiter '=' and comment '#'

    :return: created EconfFile object
    """
    result = EconfFile(c_void_p())
    err = LIBECONF.econf_newIniFile(byref(result._ptr))
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"new_ini_file failed with error: {err_string(err)}")
    return result


def comment_tag(ef: EconfFile) -> str:
    """
    Get the comment tag of the specified EconfFile

    :param ef: Key-Value storage object
    :return: The comment tag of the EconfFile
    """
    LIBECONF.econf_comment_tag.restype = c_char
    result = LIBECONF.econf_comment_tag(ef._ptr)
    return result.decode("utf-8")


def delimiter_tag(ef: EconfFile) -> str:
    """
    Get the delimiter tag of the specified EconfFile

    :param ef: Key-Value storage object
    :return: the delimiter tag of the EconfFile
    """
    LIBECONF.econf_delimiter_tag.restype = c_char
    result = LIBECONF.econf_delimiter_tag(ef._ptr)
    return result.decode("utf-8")


def set_comment_tag(ef: EconfFile, comment: str | bytes) -> None:
    """
    Set the comment tag of the specified EconfFile

    :param ef: Key-Value storage object
    :param comment: The desired comment tag character
    :return: Nothing
    """
    comment = _ensure_valid_char(comment)
    c_comment = c_char(comment)
    LIBECONF.econf_set_comment_tag(ef._ptr, c_comment)


def set_delimiter_tag(ef: EconfFile, delimiter: str | bytes) -> None:
    """
    Set the delimiter tag of the specified EconfFile

    :param ef: Key-Value storage object
    :param delimiter: The desired delimiter character
    :return: Nothing
    """
    delimiter = _ensure_valid_char(delimiter)
    c_delimiter = c_char(delimiter)
    LIBECONF.econf_set_delimiter_tag(ef._ptr, c_delimiter)


def write_file(ef: EconfFile, save_to_dir: str, file_name: str) -> None:
    """
    Write content of a keyfile to specified location

    :param ef: Key-Value storage object
    :param save_to_dir: directory into which the file has to be written
    :param file_name: filename with suffix of the to be written file
    :return: Nothing
    """
    c_save_to_dir = _encode_str(save_to_dir)
    c_file_name = _encode_str(file_name)
    err = LIBECONF.econf_writeFile(byref(ef._ptr), c_save_to_dir, c_file_name)
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"write_file failed with error: {err_string(err)}")


def get_path(ef: EconfFile) -> str:
    """
    Get the path of the source of the given key file

    :param ef: Key-Value storage object
    :return: path of the config file as string
    """
    # extract from pointer
    LIBECONF.econf_getPath.restype = c_char_p
    return LIBECONF.econf_getPath(ef._ptr).decode("utf-8")


def get_groups(ef: EconfFile) -> list[str]:
    """
    List all the groups of given keyfile

    :param ef: Key-Value storage object
    :return: list of groups in the keyfile
    """
    c_length = c_size_t()
    c_groups = c_void_p(None)
    err = LIBECONF.econf_getGroups(ef._ptr, byref(c_length), byref(c_groups))
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_groups failed with error: {err_string(err)}")
    arr = cast(c_groups, POINTER(c_char_p * c_length.value))
    result = [i.decode("utf-8") for i in arr.contents]
    return result


def get_keys(ef: EconfFile, group: str) -> list[str]:
    """
    List all the keys of a given group or all keys in a keyfile

    :param ef: Key-Value storage object
    :param group: group of the keys to be returned or None for keys without a group
    :return: list of keys in the given group
    """
    c_length = c_size_t()
    c_keys = c_void_p(None)
    if group:
        group = _encode_str(group)
    err = LIBECONF.econf_getKeys(ef._ptr, group, byref(c_length), byref(c_keys))
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_keys failed with error: {err_string(err)}")
    arr = cast(c_keys, POINTER(c_char_p * c_length.value))
    result = [i.decode("utf-8") for i in arr.contents]
    return result


def get_int_value(ef: EconfFile, group: str, key: str) -> int:
    """
    Return an integer value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_int64()
    err = LIBECONF.econf_getInt64Value(ef._ptr, group, c_key, byref(c_result))
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_int64_value failed with error: {err_string(err)}")
    return c_result.value


def get_uint_value(ef: EconfFile, group: str, key: str) -> int:
    """
    Return an unsigned integer value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_uint64()
    err = LIBECONF.econf_getUInt64Value(ef._ptr, group, c_key, byref(c_result))
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_uint64_value failed with error: {err_string(err)}")
    return c_result.value


def get_float_value(ef: EconfFile, group: str, key: str) -> float:
    """
    Return a float value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_double()
    err = LIBECONF.econf_getDoubleValue(ef._ptr, group, c_key, byref(c_result))
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_double_value failed with error: {err_string(err)}")
    return c_result.value


def get_string_value(ef: EconfFile, group: str, key: str) -> str:
    """
    Return a string value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_char_p()
    err = LIBECONF.econf_getStringValue(ef._ptr, group, c_key, byref(c_result))
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_string_value failed with error: {err_string(err)}")
    return c_result.value.decode("utf-8")


def get_bool_value(ef: EconfFile, group: str, key: str) -> bool:
    """
    Return a boolean value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_bool()
    err = LIBECONF.econf_getBoolValue(ef._ptr, group, c_key, byref(c_result))
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_bool_value failed with error: {err_string(err)}")
    return c_result.value


def get_int_value_def(ef: EconfFile, group: str, key: str, default: int) -> int:
    """
    Return an integer value for given group/key or return a default value if key is not found

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param default: value to be returned if no key is found
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_int64()
    c_default = _ensure_valid_int(default)
    err = LIBECONF.econf_getInt64ValueDef(
        ef._ptr, group, c_key, byref(c_result), c_default
    )
    if err and EconfError(err) != EconfError.NOKEY:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_int64_value_def failed with error: {err_string(err)}")
    return c_result.value


def get_uint_value_def(ef: EconfFile, group: str, key: str, default: int) -> int:
    """
    Return an unsigned integer value for given group/key or return a default value if key is not found

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param default: value to be returned if no key is found
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_uint64()
    c_default = _ensure_valid_uint(default)
    err = LIBECONF.econf_getUInt64ValueDef(
        ef._ptr, group, c_key, byref(c_result), c_default
    )
    if err and EconfError(err) != EconfError.NOKEY:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_uint64_value_def failed with error: {err_string(err)}")
    return c_result.value


def get_float_value_def(ef: EconfFile, group: str, key: str, default: float) -> float:
    """
    Return a float value for given group/key or return a default value if key is not found

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param default: value to be returned if no key is found
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_double()
    if not isinstance(default, float):
        raise TypeError('"default" parameter must be of type float')
    c_default = c_double(default)
    err = LIBECONF.econf_getDoubleValueDef(
        ef._ptr, group, c_key, byref(c_result), c_default
    )
    if err and EconfError(err) != EconfError.NOKEY:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_double_value_def failed with error: {err_string(err)}")
    return c_result.value


def get_string_value_def(ef: EconfFile, group: str, key: str, default: str) -> str:
    """
    Return a string value for given group/key or return a default value if key is not found

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param default: value to be returned if no key is found
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_char_p()
    c_default = _encode_str(default)
    err = LIBECONF.econf_getStringValueDef(
        ef._ptr, group, c_key, byref(c_result), c_default
    )
    if err:
        if EconfError(err) == EconfError.NOKEY:
            return c_default.decode("utf-8")
        raise ECONF_EXCEPTION[EconfError(err)](f"get_string_value_def failed with error: {err_string(err)}")
    return c_result.value.decode("utf-8")


def get_bool_value_def(ef: EconfFile, group: str, key: str, default: bool) -> bool:
    """
    Return a boolean value for given group/key or return a default value if key is not found

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param default: value to be returned if no key is found
    :return: value of the key
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_result = c_bool()
    if not isinstance(default, bool):
        raise TypeError('"value" parameter must be of type bool')
    c_default = c_bool(default)
    err = LIBECONF.econf_getBoolValueDef(
        ef._ptr, group, c_key, byref(c_result), c_default
    )
    if err and EconfError(err) != EconfError.NOKEY:
        raise ECONF_EXCEPTION[EconfError(err)](f"get_bool_value_def failed with error: {err_string(err)}")
    return c_result.value


def set_int_value(ef: EconfFile, group: str, key: str, value: int) -> None:
    """
    Setting an integer value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param value: value to be set for given key
    :return: Nothing
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_value = _ensure_valid_int(value)
    err = LIBECONF.econf_setInt64Value(ef._ptr, group, c_key, c_value)
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"set_int64_value failed with error: {err_string(err)}")


def set_uint_value(ef: EconfFile, group: str, key: str, value: int) -> None:
    """
    Setting an unsigned integer value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param value: value to be set for given key
    :return: Nothing
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_value = _ensure_valid_uint(value)
    err = LIBECONF.econf_setUInt64Value(ef._ptr, group, c_key, c_value)
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"set_uint64_value failed with error: {err_string(err)}")


def set_float_value(ef: EconfFile, group: str, key: str, value: float) -> None:
    """
    Setting a float value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param value: value to be set for given key
    :return: Nothing
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    if not isinstance(value, float):
        raise TypeError('"value" parameter must be of type float')
    c_value = c_double(value)
    err = LIBECONF.econf_setDoubleValue(ef._ptr, group, c_key, c_value)
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"set_double_value failed with error: {err_string(err)}")


def set_string_value(ef: EconfFile, group: str, key: str, value: str | bytes) -> None:
    """
    Setting a string value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param value: value to be set for given key
    :return: Nothing
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    c_value = _encode_str(value)
    err = LIBECONF.econf_setStringValue(ef._ptr, group, c_key, c_value)
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"set_string_value failed with error: {err_string(err)}")


def set_bool_value(ef: EconfFile, group: str, key: str, value: bool) -> None:
    """
    Setting a boolean value for given group/key

    :param ef: Key-Value storage object
    :param group: desired group
    :param key: key of the value that is requested
    :param value: value to be set for given key
    :return: Nothing
    """
    if group:
        group = _encode_str(group)
    c_key = _encode_str(key)
    if not isinstance(value, bool):
        raise TypeError('"value" parameter must be of type bool')
    c_value = _encode_str(str(value))
    err = LIBECONF.econf_setBoolValue(ef._ptr, group, c_key, c_value)
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"set_bool_value failed with error: {err_string(err)}")


def err_string(error: int) -> str:
    """
    Convert an error code into error message

    :param error: error code as integer
    :return: error string
    """
    if not isinstance(error, int):
        raise TypeError("Error codes must be of type int")
    c_int(error)
    LIBECONF.econf_errString.restype = c_char_p
    return LIBECONF.econf_errString(error).decode("utf-8")


def err_location() -> Tuple[str, int]:
    """
    Info about the line where an error happened

    :return: path to the last handled file and number of last handled line
    """
    c_filename = c_char_p()
    c_line_nr = c_uint64()
    LIBECONF.econf_errLocation(byref(c_filename), byref(c_line_nr))
    return c_filename.value.decode("utf-8"), c_line_nr.value


def free_file(ef: EconfFile):
    """
    Free the memory of a given keyfile

    This function is called automatically at the end of every objects lifetime and should not be used otherwise

    :param ef: EconfFile to be freed
    :return: None
    """
    if not isinstance(ef, EconfFile):
        raise TypeError("Parameter must be an EconfFile object")
    if not ef._ptr:
        return
    LIBECONF.econf_freeFile(ef._ptr)


def set_conf_dirs(dir_postfix_list: list[str]) -> None:
    """
    Set a list of directories (with order) that describe the paths where files have to be parsed

    E.G. with the given list: {"conf.d", ".d", "/", NULL} files in following directories will be parsed:
    "<default_dirs>/<config_name>.conf.d/" "<default_dirs>/<config_name>.d/" "<default_dirs>/<config_name>/"

    :param dir_postfix_list: List of directories
    :return: None
    """
    if type(dir_postfix_list) != list:
        raise TypeError("Directories must be passed as a list of strings")
    if len(dir_postfix_list) == 0:
        return
    str_arr = c_char_p * len(dir_postfix_list)
    dir_arr = str_arr()
    for i in range(len(dir_postfix_list)):
        if dir_postfix_list[i] is not None:
            dir_postfix_list[i] = _encode_str(dir_postfix_list[i])
        dir_arr[i] = c_char_p(dir_postfix_list[i])
    err = LIBECONF.econf_set_conf_dirs(dir_arr)
    if err:
        raise ECONF_EXCEPTION[EconfError(err)](f"set_conf_dirs failed with error: {err_string(err)}")
