# `LibEConf.NET`
`LibEConf.NET` is a .NET wrapper for `libeconf` written in C#. It is memory safe and uses P/Invoke to talk to the native library. The core library has no external dependencies besides the native binary for `libeconf`.

# Usage
The bindings are split into two classes: `EConf` is a static class which only contains static methods. All method calls have the same naming scheme,
if you want to access a function `econf_<functionName>`, the C# equivalent would be `EConf.<FunctionName>`.

`EConfFile` is an instance class, which contains all instance methods which work on a file. `EConfFile` also implements `IDisposable`, so you can use it inside of `using` scopes.

You can find more information about each method [here](methods.md).

## Example
_foo.ini_
```ini
[MyGroup]
MyKey=MyValue # This is a comment.
MyOtherKey=5
```
The following code reads a file, modifies both values and saves it again.
```cs
using LibEConf.NET; // Import the bindings.

EConfErr err = EConf.ReadFile(out EConfFile myFile, "foo.ini", "=", "#");   // Open the file for reading.
int result = myFile.GetValueInt("MyGroup", "MyOtherKey");                   // Get the value for "MyOtherKey" from the file.
myFile.SetValue("MyGroup", "MyOtherKey", result + 5);                       // Set the value for "MyOtherKey" to 5 + 5.
myFile.SetValue("MyGroup", "MyKey", "foobar");                              // Set "MyKey" to "foobar".
myFile.WriteFile("foo.ini");                                                // Save the file.
```
_foo.ini_ will then look like this:
```ini
[MyGroup]
MyKey=foobar
MyOtherKey=10
```

# Building from Source
To build from source open up the `LibEConf.NET.csproj` file in the .NET IDE of your choice and press _Build_.
If you're using the `dotnet CLI`, open up the folder where the project file is located in and run `dotnet build`.
You will also need to build `libeconf` and copy the resulting binaries (`.so`/`.dll`) to the output directory.
