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
    "file,context,group,key,value",
    [
        (FILE, does_not_raise(), "Group", "Bla", 1),
        (FILE, does_not_raise(), "Group", "Welcome", 1),
        (FILE, does_not_raise(), None, "foo2", 1),
        (FILE, pytest.raises(ValueError), "Group", "Bla", 99999999999999999999),
        (FILE, does_not_raise(), "New Group", "Bla", 1),
        (FILE, pytest.raises(TypeError), 7, 2, 1),
        (FILE, pytest.raises(TypeError), "Group", "Bla", "Invalid Value"),
        (FILE, pytest.raises(TypeError), "Invalid Group", "Bla", "Invalid Value"),
    ],
)
def test_set_int_value(file, context, group, key, value):
    with context:
        econf.set_int_value(file, group, key, value)
        result = econf.get_int_value(file, group, key)

        assert result == value


@pytest.mark.parametrize(
    "file,context,group,key,value",
    [
        (FILE, does_not_raise(), "Group", "Bla", 1),
        (FILE, does_not_raise(), "Group", "Welcome", 1),
        (FILE, does_not_raise(), "New Group", "Bla", 1),
        (FILE, does_not_raise(), None, "foo2", 1),
        (FILE, pytest.raises(TypeError), "Group", "Bla", -1),
        (FILE, pytest.raises(ValueError), "Group", "Bla", 99999999999999999999),
        (FILE, pytest.raises(TypeError), 7, 2, 1),
        (FILE, pytest.raises(TypeError), "Group", "Bla", "Invalid Value"),
        (FILE, pytest.raises(TypeError), "Invalid Group", "Bal", "Invalid Value"),
    ],
)
def test_set_uint_value(file, context, group, key, value):
    with context:
        econf.set_uint_value(file, group, key, value)
        result = econf.get_uint_value(file, group, key)

        assert result == value


@pytest.mark.parametrize(
    "file,context,group,key,value",
    [
        (FILE, does_not_raise(), None, "foo", 1.5),
        (FILE, does_not_raise(), "Group", "Welcome", 1.5),
        (FILE, does_not_raise(), "New Group", "Bla", 1.5),
        (FILE, does_not_raise(), "Group", "Bla", -1.5),
        (FILE, pytest.raises(TypeError), 7, 2, 1),
        (FILE, pytest.raises(TypeError), "Group", "Bla", "Invalid Value"),
        (FILE, pytest.raises(TypeError), "Group", "Bla", 1),
    ],
)
def test_set_float_value(file, context, group, key, value):
    with context:
        econf.set_float_value(file, group, key, value)
        result = econf.get_float_value(file, group, key)

        assert result == value


@pytest.mark.parametrize(
    "file,context,group,key,value",
    [
        (FILE, does_not_raise(), "Group", "Welcome", "Bye"),
        (FILE, does_not_raise(), "Group", "Bla", "1"),
        (FILE, does_not_raise(), "New Group", "Welcome", "Bye"),
        (FILE, does_not_raise(), "First Group", "Name", "\nNoname"),
        (FILE, pytest.raises(TypeError), 7, 2, 1),
        (FILE, pytest.raises(TypeError), "Group", "Bla", 1.5),
        (FILE, pytest.raises(TypeError), "Group", "Bla", True),
    ],
)
def test_set_string_value(file, context, group, key, value):
    with context:
        econf.set_string_value(file, group, key, value)
        result = econf.get_string_value(file, group, key)

        assert result == value


@pytest.mark.parametrize(
    "file,context,group,key,value",
    [
        (FILE, does_not_raise(), "Another Group", "Booleans", False),
        (FILE, does_not_raise(), "Group", "Bla", True),
        (FILE, does_not_raise(), "New Group", "Welcome", True),
        (FILE, pytest.raises(TypeError), 7, 2, 1),
        (FILE, pytest.raises(TypeError), "Group", "Bla", "Invalid Value"),
        (FILE, pytest.raises(TypeError), "Group", "Bla", ""),
    ],
)
def test_set_bool_value(file, context, group, key, value):
    with context:
        econf.set_bool_value(file, group, key, value)
        result = econf.get_bool_value(file, group, key)

        assert result == value
