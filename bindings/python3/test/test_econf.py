import pytest
import econf
from contextlib import contextmanager
from pathlib import Path
from ctypes import *


FILE = econf.read_file("test/testdata/examples/example.conf", "=", ";")
FILE2 = econf.read_file("test/testdata/examples2/example.conf", "=", "#")
INVALID_FILE = econf.read_file("test/testdata/examples/invalid.conf", ":", "#")


@contextmanager
def does_not_raise():
    yield


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


def test_read_file():
    file = "test/testdata/examples/example.conf"
    delim = "="
    comment = "#"

    ef = econf.read_file(file, delim, comment)

    assert ef._ptr != None


def test_new_key_file():
    result = econf.new_key_file("=", "#")

    assert result
    assert type(result) == econf.EconfFile


def test_new_ini_file():
    result = econf.new_ini_file()

    assert result
    assert type(result) == econf.EconfFile


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
    "ef,context,expected",
    [
        (FILE, does_not_raise(), ";"),
        (FILE2, does_not_raise(), "#"),
        (INVALID_FILE, does_not_raise(), "#"),
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
        (INVALID_FILE, does_not_raise(), ":"),
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
