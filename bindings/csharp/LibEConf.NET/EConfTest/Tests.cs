using LibEConf.NET;
using Xunit;

namespace EConfTest;

public class Tests
{
    [Fact]
    public void TestReadFile()
    {
        var err = EConf.ReadFile(out var file, "TestData/examples/example.conf", "=", "#");
        Assert.Equal(EConfErr.Success, err);
        Assert.Equal('=', file.DelimiterTag);
        Assert.Equal('#', file.CommentTag);
        Assert.NotNull(file.Path);
    }

    [Fact]
    public void TestGetValues()
    {
        var err = EConf.ReadFile(out var file, "TestData/examples/example.conf", "=", ";");
        Assert.Equal(EConfErr.Success, err);
        
        var groups = file.GetGroups();
        Assert.Equal(3, groups.Length);
        
        // Test if groups parse successfully.
        foreach (var group in groups)
        {
            foreach (var key in file.GetKeys(group))
            {
                Assert.Equal(EConfErr.Success, file.TryGetValue(group, key, out string unused));
            }
        }
        
        // Test if booleans are parsed successfully.
        Assert.Equal(EConfErr.Success,file.TryGetValue("Another Group", "Booleans", out bool b));
        Assert.True(b);
        
        // Test if strings are parsed successfully.
        Assert.Equal(EConfErr.Success, file.TryGetValue("First Group", "Name", out string s));
        Assert.Equal("Keys File Example\tthis value shows\nescaping", s);
        
        // Get keys from anonymous groups.
        Assert.Equal(EConfErr.Success, file.TryGetValue("", "foo", out double d));
        Assert.Equal(6.5, d);
        Assert.Equal(EConfErr.Success, file.TryGetValue("", "foo2", out int i));
        Assert.Equal(-6, i);
    }

    [Fact]
    public void TestInvalidFile()
    {
        var err = EConf.ReadFile(out var file, "TestData/examples/invalid.conf", "=", "#");
        
        Assert.Equal(EConfErr.MissingBracket, err);
        Assert.Equal(5UL, EConf.ErrLocation("TestData/examples/invalid.conf"));
    }
    
    [Fact]
    public void TestReadDirs()
    {
        var err = EConf.ReadDirs(out var file, "TestData/examples2", "TestData/examples", "example", ".conf", "=", "#");
        
        Assert.Equal(EConfErr.Success, err);
        Assert.Equal('=', file.DelimiterTag);
        Assert.Equal('#', file.CommentTag);
        Assert.NotNull(file.Path);
    }
    
    [Fact]
    public void TestReadDirsHistory()
    {
        var err = EConf.ReadDirsHistory(out var files, "TestData/examples2", "TestData/examples", "example", ".conf", "=", "#");
        
        Assert.Equal(EConfErr.Success, err);
        Assert.Equal(2, files.Length);
        foreach(var file in files)
        {
            Assert.Equal('=', file.DelimiterTag);
            Assert.Equal('#', file.CommentTag);
            Assert.NotNull(file.Path);
        }
    }

    [Fact]
    public void TestWriteNewFile()
    {
        const string path = "TestData/writeTest.conf";
        if (File.Exists(path))
            File.Delete(path);

        using var file = new EConfFile('=', '#');
        // Write empty file.
        file.WriteFile(path);
        Assert.True(File.Exists(path));
        
        // Write value then save.
        file.SetValue("My Group", "Test", 16.5);
        file.SetValue("Other Group", "TestBool", true);
        file.WriteFile(path);

        // Check if the file parsed all values.
        var err = EConf.ReadFile(out var file2, path, "=", "#");
        Assert.Equal(EConfErr.Success, err);
        Assert.Equal(16.5, file2.GetValueDouble("My Group", "Test"));
        Assert.True(file2.GetValueBool("Other Group", "TestBool"));
    }

    [Fact]
    public void TestToDictionary()
    {
        EConf.ReadDirs(out var file, "TestData/examples2", "TestData/examples", "example", ".conf", "=", "#");

        var dict1 = file.ToDictionary("First Group");
        Assert.Equal(7, dict1.Count);
        
        var dict2 = file.ToDictionary("Another Group");
        Assert.Equal(2, dict2.Count);
        
        // All groups and anonymous groups.
        var dictAll = file.ToDictionary(null);
        Assert.Equal(14, dictAll.Count);
        
        // Only anonymous groups.
        var dictAnon = file.ToDictionary("");
        Assert.Equal(3, dictAnon.Count);
    }
    
    [Fact]
    public void TestAppendDictionary()
    {
        EConf.ReadDirs(out var dictFile, "TestData/examples2", "TestData/examples", "example", ".conf", "=", "#");

        var dictAll = dictFile.ToDictionary(null);
        Assert.Equal(14, dictAll.Count);
        var file = new EConfFile('=', '#');
        file.AppendDictionary("", dictAll);
        Assert.Equal(14, file.GetKeys(null).Length);
    }
}
