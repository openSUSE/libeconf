import pytest
import econf
from contextlib import contextmanager
from pathlib import Path
from ctypes import *


FILE = econf.read_file("test/testdata/examples/example.conf", "=", ";")
FILE2 = econf.read_file("test/testdata/examples2/example.conf", "=", "#")


@contextmanager
def does_not_raise():
    yield


def user_function(value: str) -> bool:
    return value == "correct"


@pytest.mark.parametrize(
    "value,expected,context",
    [
        ("foo", b"foo", does_not_raise()),
        (b"foo", b"foo", does_not_raise()),
        (5, b"", pytest.raises(TypeError)),
    ],
)
def test_encode_str(value, expected, context):
    with context:
        assert econf._encode_str(value) == expected

@pytest.mark.parametrize(
    "context,value",
    [
        (does_not_raise(), "#"),
        (does_not_raise(), b'+'),
        (pytest.raises(TypeError), 3),
        (pytest.raises(ValueError), "abc")
    ]
)
def test_ensure_valid_char(context, value):
    with context:
        result = econf._ensure_valid_char(value)

        assert len(result) == 1
        assert isinstance(result, bytes)


@pytest.mark.parametrize(
    "value,context",
    [
        (5, does_not_raise()),
        (99999999999999999999, pytest.raises(ValueError)),
        ("a", pytest.raises(TypeError)),
    ],
)
def test_ensure_valid_int(value, context):
    with context:
        result = econf._ensure_valid_int(value)

        assert isinstance(result, c_int64)
        assert result.value == value


@pytest.mark.parametrize(
    "value,context",
    [
        (5, does_not_raise()),
        (99999999999999999999, pytest.raises(ValueError)),
        ("a", pytest.raises(TypeError)),
    ],
)
def test_ensure_valid_uint(value, context):
    with context:
        result = econf._ensure_valid_uint(value)

        assert isinstance(result, c_uint64)
        assert result.value >= 0
        assert result.value == value


@pytest.mark.parametrize(
    "file,context",
    [
        ("test/testdata/examples/example.conf", does_not_raise()),
        ("test/testdata/examples/invalid.conf", pytest.raises(SyntaxError)),
        ("test/testdata/examples/fakefile.conf", pytest.raises(FileNotFoundError))
    ]
)
def test_read_file(file, context):
    with context:
        result = econf.read_file(file, "=", "#")

        assert result._ptr != None
        assert econf.get_groups(result) != None
        assert econf.get_keys(result, None) != None
        assert econf.delimiter_tag(result) == "="
        assert econf.comment_tag(result) == "#"


@pytest.mark.parametrize(
    "file,context,data",
    [
        ("test/testdata/examples/example.conf", does_not_raise(), "correct"),
        ("test/testdata/examples/example.conf", pytest.raises(Exception, match="parsing callback has failed"), "wrong"),
        ("test/testdata/examples/fakefile.conf", pytest.raises(FileNotFoundError), "correct"),
        ("test/testdata/examples/invalid.conf", pytest.raises(SyntaxError), "correct")
    ]
)
def test_read_file_with_callback(file, context, data):
    with context:
        result = econf.read_file_with_callback(file, "=", "#", user_function, data)

        assert result._ptr != None
        assert econf.get_groups(result) != None
        assert econf.get_keys(result, None) != None
        assert econf.delimiter_tag(result) == "="
        assert econf.comment_tag(result) == "#"

@pytest.mark.parametrize(
    "context,delim,comment",
    [
        (does_not_raise(), "=", "#"),
        (pytest.raises(ValueError), "abc", "def"),
        (pytest.raises(TypeError), 1, 2)
    ]
)
def test_new_key_file(context, delim, comment):
    with context:
        result = econf.new_key_file(delim, comment)

        assert result
        assert type(result) == econf.EconfFile
        assert econf.delimiter_tag(result) == delim
        assert econf.comment_tag(result) == comment


def test_new_ini_file():
    result = econf.new_ini_file()

    assert result
    assert type(result) == econf.EconfFile
    assert econf.delimiter_tag(result) == "="
    assert econf.comment_tag(result) == "#"


def test_merge_files():
    result = econf.merge_files(FILE, FILE2)

    assert len(econf.get_keys(result, None)) == 4
    assert len(econf.get_keys(result, "Group")) == 3
    assert len(econf.get_groups(result)) == 3


def test_read_dirs():
    result = econf.read_dirs(
        "test/testdata/examples2/",
        "test/testdata/examples/",
        "example",
        "conf",
        "=",
        "#",
    )

    assert len(econf.get_keys(result, None)) == 3
    assert len(econf.get_keys(result, "Group")) == 4
    assert len(econf.get_groups(result)) == 3

@pytest.mark.parametrize(
    "context,data",
    [
        (does_not_raise(), "correct"),
        (pytest.raises(Exception, match="parsing callback has failed"), "wrong")
    ]
)
def test_read_dirs_with_callback(context, data):
    with context:
        usr_dir = "test/testdata/examples2/"
        etc_dir = "test/testdata/examples"
        name = "example"
        result = econf.read_dirs_with_callback(usr_dir, etc_dir, name, "conf", "=", "#", user_function, data)

        assert len(econf.get_keys(result, None)) == 3
        assert len(econf.get_keys(result, "Group")) == 4
        assert len(econf.get_groups(result)) == 3


def test_read_dirs_history():
    result = econf.read_dirs_history(
        "test/testdata/examples2/",
        "test/testdata/examples/",
        "example",
        "conf",
        "=",
        "#",
    )

    assert len(result) == 2
    assert len(econf.get_groups(result[0])) == 3
    assert len(econf.get_keys(result[0], None)) == 2
    assert len(econf.get_groups(result[1])) == 1

@pytest.mark.parametrize(
    "context,data",
    [
        (does_not_raise(), "correct"),
        (pytest.raises(Exception, match="parsing callback has failed"), "wrong")
    ]
)
def test_read_dirs_history_with_callback(context, data):
    with context:
        usr_dir = "test/testdata/examples2/"
        etc_dir = "test/testdata/examples"
        name = "example"
        result = econf.read_dirs_history_with_callback(usr_dir, etc_dir, name, "conf", "=", "#", user_function, data)

        assert len(result) == 2
        assert len(econf.get_groups(result[0])) == 3
        assert len(econf.get_keys(result[0], None)) == 2
        assert len(econf.get_groups(result[1])) == 1


@pytest.mark.parametrize(
    "ef,context,expected",
    [
        (FILE, does_not_raise(), ";"),
        (FILE2, does_not_raise(), "#"),
    ],
)
def test_comment_tag(ef, context, expected):
    with context:
        result = econf.comment_tag(ef)

        assert result == expected


@pytest.mark.parametrize(
    "ef,context,expected",
    [
        (FILE, does_not_raise(), "="),
        (FILE2, does_not_raise(), "="),
    ],
)
def test_delimiter_tag(ef, context, expected):
    with context:
        result = econf.delimiter_tag(ef)

        assert result == expected


@pytest.mark.parametrize(
    "ef,context,expected",
    [
        (FILE, does_not_raise(), "/"),
        (FILE, pytest.raises(TypeError), 1),
        (FILE, pytest.raises(ValueError), "abc"),
    ],
)
def test_set_comment_tag(ef, context, expected):
    with context:
        econf.set_comment_tag(ef, expected)
        result = econf.comment_tag(ef)

        assert result == expected


@pytest.mark.parametrize(
    "ef,context, expected",
    [
        (FILE, does_not_raise(), ":"),
        (FILE, pytest.raises(TypeError), 1),
        (FILE, pytest.raises(ValueError), "abc"),
    ],
)
def test_set_delimiter_tag(ef, context, expected):
    with context:
        econf.set_delimiter_tag(ef, expected)
        result = econf.delimiter_tag(ef)

        assert result == expected


def test_write_file(tmp_path):
    d = str(tmp_path)
    name = "example.conf"
    result = econf.write_file(FILE, d, name)

    assert (tmp_path / "example.conf").exists()


@pytest.mark.parametrize(
    "context,value,expected",
    [
        (does_not_raise(), 0, "Success"),
        (does_not_raise(), 5, "Key not found"),
        (does_not_raise(), 23, "Unknown libeconf error 23"),
        (pytest.raises(TypeError), "", "")
    ]
)
def test_err_string(context, value, expected):
    with context:
        result = econf.err_string(value)

        assert result == expected


def test_err_location():
    file, line = econf.err_location()

    assert isinstance(file, str)
    assert isinstance(line, int)


@pytest.mark.parametrize(
    "file,context",
    [
        #(FILE, does_not_raise()),
        (econf.EconfFile(c_void_p(None)), does_not_raise()),
        (5, pytest.raises(TypeError))
    ]
)
def test_free_file(file, context):
    with context:
        econf.free_file(file)

@pytest.mark.parametrize(
    "context,list",
    [
        (does_not_raise(), ["/", "/conf.d/", None]),
        (does_not_raise(), []),
        (pytest.raises(TypeError), "")
    ]
)
def test_set_conf_dirs(context, list):
    with context:
        econf.set_conf_dirs(list)
