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


@pytest.mark.parametrize(
    "file,context,group,key,expected",
    [
        (FILE, does_not_raise(), "Group", "Bla", 12311),
        (FILE, does_not_raise(), "Group", "Invalid Key", 1),
        (FILE, does_not_raise(), "Invalid Group", "Bla", 1),
        (FILE, does_not_raise(), None, "foo", 6),
        (FILE, does_not_raise(), "Group", "Welcome", 0),
        (FILE, pytest.raises(TypeError), 7, 2, 12311),
        (FILE, pytest.raises(TypeError), "Group", "Bla", "default"),
        (FILE, pytest.raises(TypeError), "Group", "Invalid Key", "default"),
    ],
)
def test_get_int_value_def(file, context, group, key, expected):
    with context:
        result = econf.get_int_value_def(file, group, key, expected)

        assert isinstance(result, int)
        assert result == expected


@pytest.mark.parametrize(
    "file,context,group,key,expected",
    [
        (FILE, does_not_raise(), "Group", "Bla", 12311),
        (FILE, does_not_raise(), "Group", "Invalid Key", 0),
        (FILE, does_not_raise(), "Invalid Group", "Bla", 12311),
        (FILE, does_not_raise(), None, "foo", 6),
        (FILE, does_not_raise(), "Group", "Welcome", 0),
        (FILE, pytest.raises(TypeError), 7, 2, 12311),
        (FILE, pytest.raises(TypeError), "Group", "Bla", "default"),
        (FILE, pytest.raises(TypeError), "Group", "Invalid Key", "default"),
    ],
)
def test_get_uint_value_def(file, context, group, key, expected):
    with context:
        result = econf.get_uint_value_def(file, group, key, expected)

        assert isinstance(result, int)
        assert result >= 0
        assert result == expected


@pytest.mark.parametrize(
    "file,context,group,key,expected",
    [
        (FILE, does_not_raise(), "Group", "Bla", 12311.0),
        (FILE, does_not_raise(), "Group", "Invalid Key", 0.1),
        (FILE, does_not_raise(), "Invalid Group", "Bla", 12311.1),
        (FILE, does_not_raise(), None, "foo", 6.5),
        (FILE, does_not_raise(), "Group", "Welcome", 0.0),
        (FILE, pytest.raises(TypeError), 7, 2, 12311),
        (FILE, pytest.raises(TypeError), None, "foo", "default"),
        (FILE, pytest.raises(TypeError), "Group", "Invalid Key", "default"),
    ],
)
def test_get_float_value_def(file, context, group, key, expected):
    with context:
        result = econf.get_float_value_def(file, group, key, expected)

        assert isinstance(result, float)
        assert result >= 0
        assert result == expected


@pytest.mark.parametrize(
    "file,context,group,key,expected",
    [
        (FILE, does_not_raise(), "Group", "Welcome", "Hello"),
        (
            FILE,
            does_not_raise(),
            "First Group",
            "Name",
            "Keys File Example\\tthis value shows\\nescaping",
        ),
        (FILE, does_not_raise(), "First Group", "Welcome[de]", "Hallo"),
        (FILE, does_not_raise(), "Group", "Bla", "12311"),
        (FILE, does_not_raise(), None, "foo", "6.5"),
        (FILE, does_not_raise(), "Invalid Group", "Bla", "default"),
        (FILE, does_not_raise(), "Group", "foo", "default"),
        (FILE, pytest.raises(TypeError), 7, 2, 12311),
        (FILE, pytest.raises(TypeError), "Group", "Welcome", 7),
        (FILE, pytest.raises(TypeError), "Group", "Invalid Key", 7),
    ],
)
def test_get_string_value_def(file, context, group, key, expected):
    with context:
        result = econf.get_string_value_def(file, group, key, expected)

        assert isinstance(result, str)
        assert result == expected


@pytest.mark.parametrize(
    "file,context,group,key,expected",
    [
        (FILE, does_not_raise(), "Another Group", "Booleans", True),
        (FILE, pytest.raises(Exception, match="Parse error"), "Group", "Bla", True),
        (FILE, does_not_raise(), "Invalid Group", "Booleans", False),
        (FILE, does_not_raise(), "Another Group", "Bools", False),
        (FILE, pytest.raises(TypeError), 7, 2, 12311),
        (FILE, pytest.raises(TypeError), "Another Group", "Booleans", 12311),
        (FILE, pytest.raises(TypeError), "Another Group", "Bools", 12311),
    ],
)
def test_get_bool_value_def(file, context, group, key, expected):
    with context:
        result = econf.get_bool_value_def(file, group, key, expected)

        assert isinstance(result, bool)
        assert result == expected
