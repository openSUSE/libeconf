using System.Runtime.InteropServices;
using System.Text;

namespace LibEConf.NET;

public static class StringExtensions
{
    /// <summary>
    /// Copies a string as UTF-8 on the global heap.
    /// </summary>
    /// <param name="s">The string to convert.</param>
    /// <returns>A pointer to the unmanaged data.</returns>
    public static nint   ToHGlobalUni8(this string s)
    {
        // Convert to UTF8
        var b = Encoding.UTF8.GetBytes(s);
        // Allocate memory and copy to pointer.
        var ptr = Marshal.AllocHGlobal(b.Length + 1);
        Marshal.Copy(b, 0, ptr, b.Length);
        // Set the last byte to 0.
        unsafe
        {
            ((byte*)ptr)[b.Length] = 0;
        }

        return ptr;
    }
}