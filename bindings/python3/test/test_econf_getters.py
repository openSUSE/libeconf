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
    "file,context,example",
    [
        (FILE, does_not_raise(), ["Another Group", "First Group", "Group"]),
        (FILE2, pytest.raises(KeyError), []),
    ],
)
def test_get_groups(file, context, example):
    with context:
        assert econf.get_groups(file) == example


@pytest.mark.parametrize(
    "file,context,group,expected",
    [
        (FILE, does_not_raise(), "Group", 3),
        (FILE, does_not_raise(), None, 2),
        (FILE2, does_not_raise(), None, 2),
        (FILE, pytest.raises(TypeError), 1, 0),
        (FILE, pytest.raises(KeyError), "a", 0),
    ],
)
def test_get_keys(file, context, group, expected):
    with context:
        result = econf.get_keys(file, group)

        assert len(result) == expected


@pytest.mark.parametrize(
    "file,context,group,key,expected",
    [
        (FILE, does_not_raise(), "Group", "Bla", 12311),
        (FILE, pytest.raises(KeyError), "Group", "a", 0),
        (FILE, pytest.raises(KeyError), "a", "Bla", 12311),
        (FILE, pytest.raises(TypeError), 7, 2, 12311),
        (FILE, does_not_raise(), None, "foo", 6),
        (FILE, does_not_raise(), "Group", "Welcome", 0),
    ],
)
def test_get_int_value(file, context, group, key, expected):
    with context:
        result = econf.get_int_value(file, group, key)

        assert isinstance(result, int)
        assert result == expected


@pytest.mark.parametrize(
    "file,context,group,key,expected",
    [
        (FILE, does_not_raise(), "Group", "Bla", 12311),
        (FILE, pytest.raises(KeyError), "Group", "a", 0),
        (FILE, pytest.raises(KeyError), "a", "Bla", 12311),
        (FILE, pytest.raises(TypeError), 7, 2, 12311),
        (FILE, does_not_raise(), None, "foo", 6),
        (FILE, does_not_raise(), "Group", "Welcome", 0),
    ],
)
def test_get_uint_value(file, context, group, key, expected):
    with context:
        result = econf.get_uint_value(file, group, key)

        assert isinstance(result, int)
        assert result >= 0
        assert result == expected


@pytest.mark.parametrize(
    "file,context,group,key,expected",
    [
        (FILE, does_not_raise(), "Group", "Bla", 12311),
        (FILE, pytest.raises(KeyError), "Group", "a", 0),
        (FILE, pytest.raises(KeyError), "a", "Bla", 12311),
        (FILE, pytest.raises(TypeError), 7, 2, 12311),
        (FILE, does_not_raise(), None, "foo", 6.5),
        (FILE, does_not_raise(), "Group", "Welcome", 0),
    ],
)
def test_get_float_value(file, context, group, key, expected):
    with context:
        result = econf.get_float_value(file, group, key)

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
        (FILE, pytest.raises(KeyError), "a", "Bla", "12311"),
        (FILE, pytest.raises(KeyError), "Group", "foo", "6.5"),
        (FILE, pytest.raises(TypeError), 7, 2, "12311"),
    ],
)
def test_get_string_value(file, context, group, key, expected):
    with context:
        result = econf.get_string_value(file, group, key)

        assert isinstance(result, str)
        assert result == expected


@pytest.mark.parametrize(
    "file,context,group,key,expected",
    [
        (FILE, does_not_raise(), "Another Group", "Booleans", True),
        (FILE, pytest.raises(Exception, match="Parse error"), "Group", "Bla", True),
        (FILE, pytest.raises(KeyError), "a", "Booleans", True),
        (FILE, pytest.raises(KeyError), "Another Group", "Bools", True),
        (FILE, pytest.raises(TypeError), 7, 2, 12311),
    ],
)
def test_get_bool_value(file, context, group, key, expected):
    with context:
        result = econf.get_bool_value(file, group, key)

        assert isinstance(result, bool)
        assert result == expected
