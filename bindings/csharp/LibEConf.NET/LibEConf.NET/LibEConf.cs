using System.Runtime.InteropServices;
using System.Text.RegularExpressions;

namespace LibEConf.NET;

public enum EConfErr
{
    Success = 0,
    Error = 1,
    NoMem = 2,
    NoFile = 3,
    NoGroup = 4,
    NoKey = 5,
    EmptyKey = 6,
    WriteError = 7,
    ParseError = 8,
    MissingBracket = 9,
    MissingDelimiter = 10,
    EmptySectionName = 11,
    TextAfterSelection = 12,
    FileListIsNull = 13,
    WrongBooleanValue = 14,
    KeyHasNullValue = 15,
    WrongOwner = 16,
    WrongGroup = 17,
    WrongFilePermission = 18,
    WrongDirPermission = 19,
    ErrorFileIsSymLink = 20,
    ParsingCallbackFailed = 21
}

public class EConfFile : IDisposable
{
    internal nint Internal;

    /// <summary>
    /// Gets or sets the comment character for this file.
    /// </summary>
    public char CommentTag
    {
        get => (char)EConf.econf_comment_tag(Internal);
        set => EConf.econf_set_comment_tag(Internal, (sbyte)value);
    }
    
    /// <summary>
    /// Gets or sets the delimiter character for this file.
    /// </summary>
    public char DelimiterTag
    {
        get => (char)EConf.econf_delimiter_tag(Internal);
        set => EConf.econf_set_delimiter_tag(Internal, (sbyte)value);
    }
    
    /// <summary>
    /// Gets the path of this file.
    /// </summary>
    public string? Path => Marshal.PtrToStringUTF8(EConf.econf_getPath(Internal));

    /// <summary>
    /// Creates a new empty `.ini` file instance.
    /// The delimiter will be "=" and comments are beginning with "#".
    /// </summary>
    public EConfFile()
    {
        unsafe
        {
            nint p;
            EConf.econf_newIniFile((nint)(&p));
            Internal = p;
        }
    }
    
    /// <summary>
    /// Creates a new empty file instance with the given delimiter and comment character.
    /// </summary>
    /// <param name="delimiter">Delimiter of key/value e.g. "=".</param>
    /// <param name="comment">Character which defines the start of a comment.</param>
    public EConfFile(char delimiter, char comment)
    {
        unsafe
        {
            nint p;
            EConf.econf_newKeyFile((nint)(&p), (sbyte)delimiter, (sbyte)comment);
            Internal = p;
        }
    }
    
    internal EConfFile(nint ptr)
    {
        Internal = ptr;
    }
    
    /// <summary>
    /// Write content of a <see cref="EConfFile"/> class to the specified location.
    /// </summary>
    /// <param name="fileName">File path to save to.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr WriteFile(string fileName)
    {
        var folderPtr = System.IO.Path.GetDirectoryName(fileName)!.ToHGlobalUni8();
        var filePtr = System.IO.Path.GetFileName(fileName).ToHGlobalUni8();
        var err = EConf.econf_writeFile(Internal, folderPtr, filePtr);
        Marshal.FreeHGlobal(folderPtr);
        Marshal.FreeHGlobal(filePtr);
        return err;
    }
    
    /// <summary>
    /// Gets all groups in this file.
    /// </summary>
    /// <returns>The names of all groups in this file.</returns>
    public string[] GetGroups()
    {
        EConfErr err;
        nint len, groups = 0;
        unsafe
        {
            err = EConf.econf_getGroups(Internal, (nint)(&len), (nint)(&groups));
        }
        if (err != EConfErr.Success) return Array.Empty<string>();
        var result = new string[len];
        unsafe
        {
            for (var i = 0; i < len; i++)
            {
                result[i] = new string(((sbyte**)groups)[i]);
            }
            
        }
        return result;
    }
    
    /// <summary>
    /// Gets all keys for a specified group in this file.
    /// </summary>
    /// <param name="group">The group to get the keys from, an empty string for all anonymous keys, or null for all keys.</param>
    /// <returns>The keys in the specified group.</returns>
    public string[] GetKeys(string? group)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        EConfErr err;
        nint len, keys = 0;
        unsafe
        {
            err = EConf.econf_getKeys(Internal, groupPtr, (nint)(&len), (nint)(&keys));
        }
        if (err != EConfErr.Success) return Array.Empty<string>();
        var result = new string[len];
        unsafe
        {
            for (var i = 0; i < len; i++)
            {
                result[i] = new string(((sbyte**)keys)[i]);
            }
        }
        
        Marshal.FreeHGlobal(groupPtr);
        return result;
    }
    
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="result"/>
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="result">If successful, the read value. Otherwise the default value.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr TryGetValue(string? group, string key, out int result)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();

        result = default;
        EConfErr err;
        unsafe
        {
            fixed (int* p = &result)
            {
                err = EConf.econf_getIntValue(Internal, groupPtr, keyPtr, (nint)p);
            }
        }
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);

        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="result"/>
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="result">If successful, the read value. Otherwise the default value.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr TryGetValue(string? group, string key, out uint result)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();

        result = default;
        EConfErr err;
        unsafe
        {
            fixed (uint* p = &result)
            {
                err = EConf.econf_getUIntValue(Internal, groupPtr, keyPtr, (nint)p);
            }
        }
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);

        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="result"/>
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="result">If successful, the read value. Otherwise the default value.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr TryGetValue(string? group, string key, out long result)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();

        result = default;
        EConfErr err;
        unsafe
        {
            fixed (long* p = &result)
            {
                err = EConf.econf_getInt64Value(Internal, groupPtr, keyPtr, (nint)p);
            }
        }
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);

        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="result"/>
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="result">If successful, the read value. Otherwise the default value.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr TryGetValue(string? group, string key, out ulong result)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();

        result = default;
        EConfErr err;
        unsafe
        {
            fixed (ulong* p = &result)
            {
                err = EConf.econf_getUInt64Value(Internal, groupPtr, keyPtr, (nint)p);
            }
        }
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);

        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="result"/>
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="result">If successful, the read value. Otherwise the default value.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr TryGetValue(string? group, string key, out float result)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();

        result = default;
        EConfErr err;
        unsafe
        {
            fixed (float* p = &result)
            {
                err = EConf.econf_getFloatValue(Internal, groupPtr, keyPtr, (nint)p);
            }
        }
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);

        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="result"/>
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="result">If successful, the read value. Otherwise the default value.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr TryGetValue(string? group, string key, out double result)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();

        result = default;
        EConfErr err;
        unsafe
        {
            fixed (double* p = &result)
            {
                err = EConf.econf_getDoubleValue(Internal, groupPtr, keyPtr, (nint)p);
            }
        }
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);

        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="result"/>
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="result">If successful, the read value. Otherwise an empty string.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr TryGetValue(string? group, string key, out string result)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();
        
        EConfErr err;
        unsafe
        {
            nint resultPtr = 0;
            err = EConf.econf_getStringValue(Internal, groupPtr, keyPtr, (nint)(&resultPtr));
            result = err == EConfErr.Success ? Regex.Unescape(new string((sbyte*)resultPtr)) : string.Empty;
        }
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);

        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="result"/>
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="result">If successful, the read value. Otherwise the default value.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr TryGetValue(string? group, string key, out bool result)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();

        result = default;
        EConfErr err;
        unsafe
        {
            fixed (bool* p = &result)
            {
                err = EConf.econf_getBoolValue(Internal, groupPtr, keyPtr, (nint)p);
            }
        }
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);

        return err;
    }

    /// <summary>
    /// Gets the value for the specified key in the specified group.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <returns>The value of the specified key.</returns>
    public int GetValueInt(string? group, string key)
    {
        TryGetValue(group, key, out int result);
        return result;
    }
    /// <summary>
    /// Gets the value for the specified key in the specified group.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <returns>The value of the specified key.</returns>
    public uint GetValueUInt(string? group, string key)
    {
        TryGetValue(group, key, out uint result);
        return result;
    }
    /// <summary>
    /// Gets the value for the specified key in the specified group.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <returns>The value of the specified key.</returns>
    public long GetValueInt64(string? group, string key)
    {
        TryGetValue(group, key, out long result);
        return result;
    }
    /// <summary>
    /// Gets the value for the specified key in the specified group.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <returns>The value of the specified key.</returns>
    public ulong GetValueUInt64(string? group, string key)
    {
        TryGetValue(group, key, out ulong result);
        return result;
    }
    /// <summary>
    /// Gets the value for the specified key in the specified group.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <returns>The value of the specified key.</returns>
    public float GetValueFloat(string? group, string key)
    {
        TryGetValue(group, key, out float result);
        return result;
    }
    /// <summary>
    /// Gets the value for the specified key in the specified group.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <returns>The value of the specified key.</returns>
    public double GetValueDouble(string? group, string key)
    {
        TryGetValue(group, key, out double result);
        return result;
    }
    /// <summary>
    /// Gets the value for the specified key in the specified group.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <returns>The value of the specified key.</returns>
    public string GetValueString(string? group, string key)
    {
        TryGetValue(group, key, out string result);
        return result;
    }
    /// <summary>
    /// Gets the value for the specified key in the specified group.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group.</param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <returns>The value of the specified key.</returns>
    public bool GetValueBool(string? group, string key)
    {
        TryGetValue(group, key, out bool result);
        return result;
    }
    
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="value"/>.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group. </param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="value">The value to set.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr SetValue(string? group, string key, int value)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();
        
        var err = EConf.econf_setIntValue(Internal, groupPtr, keyPtr, value);
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);
        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="value"/>.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group. </param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="value">The value to set.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr SetValue(string? group, string key, uint value)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();
        
        var err = EConf.econf_setUIntValue(Internal, groupPtr, keyPtr, value);
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);
        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="value"/>.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group. </param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="value">The value to set.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr SetValue(string? group, string key, long value)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();
        
        var err = EConf.econf_setInt64Value(Internal, groupPtr, keyPtr, value);
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);
        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="value"/>.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group. </param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="value">The value to set.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr SetValue(string? group, string key, ulong value)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();
        
        var err = EConf.econf_setUInt64Value(Internal, groupPtr, keyPtr, value);
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);
        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="value"/>.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group. </param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="value">The value to set.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr SetValue(string? group, string key, float value)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();
        
        var err = EConf.econf_setFloatValue(Internal, groupPtr, keyPtr, value);
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);
        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="value"/>.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group. </param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="value">The value to set.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr SetValue(string? group, string key, double value)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();
        
        var err = EConf.econf_setDoubleValue(Internal, groupPtr, keyPtr, value);
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);
        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="value"/>.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group. </param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="value">The value to set.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr SetValue(string? group, string key, string value)
    {
        if (value is null) throw new ArgumentNullException(nameof(value));
        // Escape the string if we have to.
        value = value.Escape();
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();
        var strPtr = value.ToHGlobalUni8();
        var err = EConf.econf_setStringValue(Internal, groupPtr, keyPtr, strPtr);
        Marshal.FreeHGlobal(strPtr);
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);
        return err;
    }
    /// <summary>
    /// Attempts to get a value from the specified key and attempts to store it in <paramref name="value"/>.
    /// </summary>
    /// <param name="group">The group of the key, an empty string or null for an anonymous group. </param>
    /// <param name="key">The name of the key. May not be null.</param>
    /// <param name="value">The value to set.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public EConfErr SetValue(string? group, string key, bool value)
    {
        var groupPtr = group?.ToHGlobalUni8() ?? 0;
        var keyPtr = key.ToHGlobalUni8();
        
        var strPtr = value.ToString().ToLower().ToHGlobalUni8();
        var err = EConf.econf_setBoolValue(Internal, groupPtr, keyPtr, strPtr);
        Marshal.FreeHGlobal(strPtr);
        
        Marshal.FreeHGlobal(groupPtr);
        Marshal.FreeHGlobal(keyPtr);
        return err;
    }

    /// <summary>
    /// Converts all KeyValue pairs of a group to a dictionary.
    /// </summary>
    /// <param name="group">The group to focus on. An empty string for anonymous groups, or null for all groups.</param>
    /// <returns>The selected KeyValue pairs.</returns>
    public Dictionary<string, string> ToDictionary(string? group)
    {
        var result = new Dictionary<string, string>();
        if (group is null)
        {
            foreach (var k in GetKeys(null))
                result[k] = GetValueString(null, k);
            
            foreach (var g in GetGroups())
            {
                foreach (var k in GetKeys(g))
                    result[k] = GetValueString(g, k);
            }
        }
        else
        {
            foreach (var k in GetKeys(group))
                result[k] = GetValueString(group, k);
        }
            
        return result;
    }

    /// <summary>
    /// Appends all KeyValue pairs of <paramref name="dict"/> to the file.
    /// </summary>
    /// <param name="group">The group to append to, or an empty string/null for anonymous group.</param>
    /// <param name="dict">The data to append.</param>
    public void AppendDictionary(string? group, Dictionary<string, string> dict)
    {
        if (dict.Count == 0) return;
        foreach (var kvp in dict)
        {
            SetValue(group, kvp.Key, kvp.Value);
        }
    }
    
    /// <summary>
    /// Releases all resources associated with this file.
    /// </summary>
    public void Dispose()
    {
        GC.SuppressFinalize(this);
        if (Internal == 0) throw new ObjectDisposedException(nameof(EConfFile));
        EConf.econf_freeFile(Internal);
        Internal = 0;
    }
}

public static partial class EConf
{
    private const string EConfLibName = "econf";
    
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_readFile(nint result, nint fileName, nint delim, nint comment);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_mergeFiles(nint mergedFile, nint usrFile, nint etcFile);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_readDirs(nint keyFile, nint usrConfDir, nint etcConfDir, nint projectName, nint configSuffix, nint delim, nint comment);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_readDirsHistory(nint keyFiles, nint size, nint usrConfDir, nint etcConfDir, nint projectName, nint configSuffix, nint delim, nint comment);
    [LibraryImport(EConfLibName)] internal static partial sbyte econf_comment_tag(nint keyFile);
    [LibraryImport(EConfLibName)] internal static partial sbyte econf_delimiter_tag(nint keyFile);
    [LibraryImport(EConfLibName)] internal static partial void econf_set_comment_tag(nint keyFile, sbyte comment);
    [LibraryImport(EConfLibName)] internal static partial void econf_set_delimiter_tag(nint keyFile, sbyte delimiter);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_writeFile(nint keyFile, nint saveToDir, nint fileName);
    [LibraryImport(EConfLibName)] internal static partial nint econf_getPath(nint kf);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getGroups(nint kf, nint length, nint groups);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getKeys(nint kf, nint group, nint length, nint keys);
    [LibraryImport(EConfLibName)] internal static partial void econf_errLocation(nint fileName, nint lineNr);
    [LibraryImport(EConfLibName)] internal static partial void econf_freeArray(nint array);
    [LibraryImport(EConfLibName)] internal static partial void econf_freeFile(nint file);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_set_conf_dirs(nint file);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_newKeyFile(nint result, sbyte delimiter, sbyte comment);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_newIniFile(nint result);
    
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getIntValue(nint kf, nint group, nint key, nint result);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getInt64Value(nint kf, nint group, nint key, nint result);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getUIntValue(nint kf, nint group, nint key, nint result);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getUInt64Value(nint kf, nint group, nint key, nint result);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getFloatValue(nint kf, nint group, nint key, nint result);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getDoubleValue(nint kf, nint group, nint key, nint result);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getStringValue(nint kf, nint group, nint key, nint result);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_getBoolValue(nint kf, nint group, nint key, nint result);
    
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_setIntValue(nint kf, nint group, nint key, int value);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_setInt64Value(nint kf, nint group, nint key, long value);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_setUIntValue(nint kf, nint group, nint key, uint value);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_setUInt64Value(nint kf, nint group, nint key, ulong value);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_setFloatValue(nint kf, nint group, nint key, float value);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_setDoubleValue(nint kf, nint group, nint key, double value);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_setStringValue(nint kf, nint group, nint key, nint value);
    [LibraryImport(EConfLibName)] internal static partial EConfErr econf_setBoolValue(nint kf, nint group, nint key, nint value);
    
    /// <summary>
    /// Process the file of the given fileName and save its contents into result object.
    /// </summary>
    /// <param name="result">Content of parsed file.</param>
    /// <param name="fileName">Absolute path of parsed file.</param>
    /// <param name="delim">Delimiters of key/value e.g. "\t =".
    /// If delim contains space characters AND none space characters, multiline values are not parseable.</param>
    /// <param name="comment">Array of characters which define the start of a comment.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public static EConfErr ReadFile(out EConfFile result, string fileName, string delim, string comment)
    {
        EConfErr err;
        nint file;
        
        var fileNamePtr = fileName.ToHGlobalUni8();
        var delimPtr = delim.ToHGlobalUni8();
        var commentPtr = comment.ToHGlobalUni8();
        
        unsafe
        {
            err = econf_readFile((nint)(&file), fileNamePtr, delimPtr, commentPtr);
        }

        result = new EConfFile(file);
        return err;
    }

    /// <summary>
    /// Merge the contents of two key_files objects. Entries in <paramref name="etcFile"/> will be preferred.
    /// Comment and delimiter tag will be taken from <paramref name="usrFile"/>. This can be changed
    /// by calling the functions <see cref="EConfFile.CommentTag"/> and <see cref="EConfFile.DelimiterTag"/>.
    /// </summary>
    /// <param name="mergedFile">Merged data.</param>
    /// <param name="usrFile">First data block which has to be merged.</param>
    /// <param name="etcFile">Second data block which has to be merged.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public static EConfErr MergeFiles(out EConfFile mergedFile, EConfFile usrFile, EConfFile etcFile)
    {
        mergedFile = new EConfFile();
        nint result;
        EConfErr err;
        
        unsafe
        { 
            err = econf_mergeFiles((nint)(&result), usrFile.Internal, etcFile.Internal);
        }

        mergedFile = new EConfFile(result);
        return err;
    }

    /// <summary>
    /// Evaluating key/values of a given configuration by reading and merging all
    /// needed/available files in two different directories (normally in /usr/etc and /etc).
    /// </summary>
    /// <param name="keyFile">Content of the parsed file(s).</param>
    /// <param name="usrConfDir">Absolute path of the first directory (normally "/usr/etc").</param>
    /// <param name="etcConfDir">Absolute path of the second directory (normally "/etc").</param>
    /// <param name="projectName">Base name of the configuration file.</param>
    /// <param name="configSuffix">Suffix of the configuration file. Can also be null.</param>
    /// <param name="delim">Delimiters of key/value e.g. "\t ="
    /// If delim contains space characters AND none space characters, multiline values are not parseable.</param>
    /// <param name="comment">String which defines the start of a comment.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public static EConfErr ReadDirs(out EConfFile keyFile, string usrConfDir, string etcConfDir, string projectName, string? configSuffix, string delim, string comment)
    {
        nint result;
        EConfErr err;
        
        unsafe
        {
            err = econf_readDirs((nint)(&result), usrConfDir.ToHGlobalUni8(), etcConfDir.ToHGlobalUni8(), projectName.ToHGlobalUni8(), configSuffix?.ToHGlobalUni8() ?? 0, delim.ToHGlobalUni8(), comment.ToHGlobalUni8());
        }

        keyFile = new EConfFile(result);
        return err;
    }

    /// <summary>
    /// Evaluating key/values for every given configuration files in two different
    /// directories (normally in /usr/etc and /etc). Returns a list of read configuration
    /// files and their values.
    /// </summary>
    /// <param name="keyFiles">Array of parsed file(s). Each entry includes all key/value, path, comments,...
    /// information of the regarding file.</param>
    /// <param name="userConfDir">Absolute path of the first directory (normally "/usr/etc").</param>
    /// <param name="etcConfDir">Absolute path of the second directory (normally "/etc").</param>
    /// <param name="projectName">Base name of the configuration file.</param>
    /// <param name="configSuffix">Suffix of the configuration file. Can also be null.</param>
    /// <param name="delim">Delimiters of key/value e.g. "\t ="
    /// If delim contains space characters AND none space characters, multiline values are not parseable.</param>
    /// <param name="comment">String which defines the start of a comment.</param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public static EConfErr ReadDirsHistory(out EConfFile[] keyFiles, string userConfDir, string etcConfDir, string projectName, string? configSuffix, string delim, string comment)
    {
        EConfErr err;

        unsafe
        {
            nint* keyFilesPtr;
            nint keyFilesSize;
            err = econf_readDirsHistory((nint)(&keyFilesPtr), (nint)(&keyFilesSize), userConfDir.ToHGlobalUni8(),
                etcConfDir.ToHGlobalUni8(), projectName.ToHGlobalUni8(), configSuffix?.ToHGlobalUni8() ?? 0,
                delim.ToHGlobalUni8(), comment.ToHGlobalUni8());

            keyFiles = new EConfFile[keyFilesSize];
            for (var i = 0; i < keyFilesSize; i++)
            {
                keyFiles[i] = new EConfFile()
                {
                    Internal = keyFilesPtr[i]
                };
            }
        }
        
        return err;
    }

    /// <summary>
    /// Set a list of directory structures (with order) which describes
    /// the directories in which the files have to be parsed.
    /// </summary>
    /// <param name="dirPostFixList">List of directory structures.
    /// E.G. with the given list: {"/conf.d/", ".d/", "/"} files in following directories will be parsed:
    ///    "&lt;default_dirs&gt;/&lt;project_name&gt;.&lt;suffix&gt;.d/"
    ///    "&lt;default_dirs&gt;/&lt;project_name&gt;/conf.d/"
    ///    "&lt;default_dirs&gt;/&lt;project_name&gt;.d/"
    ///    "&lt;default_dirs&gt;/&lt;project_name&gt;/".
    /// The entry "&lt;default_dirs&gt;/&lt;project_name&gt;.&lt;suffix&gt;.d/" will be added automatically.
    /// </param>
    /// <returns><see cref="EConfErr.Success"/> or error code.</returns>
    public static EConfErr SetConfDirs(string[] dirPostFixList)
    {
        unsafe
        {
            var bufferPtr = (char**)Marshal.AllocHGlobal(sizeof(nint) * (dirPostFixList.Length + 1));
            for (var i = 0; i < dirPostFixList.Length; i++)
            {
                bufferPtr[i] = (char*)dirPostFixList[i].ToHGlobalUni8();
            }
            
            return econf_set_conf_dirs((nint)bufferPtr);
        }
    }

    /// <summary>
    /// Info about where the error has happened.
    /// </summary>
    /// <param name="path">Path of the last scanned file.</param>
    /// <returns>Number of the last handled line.</returns>
    public static ulong ErrLocation(string path)
    {
        unsafe
        {
            ulong line;
            var s = path.ToHGlobalUni8();
            econf_errLocation(s, (nint)(&line));
            Marshal.FreeHGlobal(s);
            return line;
        }
    }
}
