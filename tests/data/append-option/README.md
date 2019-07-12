Append options
--------------
Append options are an extension to list options:

* Existing values in the list are not overwritten, but the new values are appended instead.
* Appending an empty value resets the list.


Example:

    key =
        one
        two
    
    -> {"key": ["one", "two"]}


    key =
        three
        four
    
    -> {"key": ["one", "two", "three", "four"]}


    key = 
    
    -> {"key": []}


    key = five
    
    -> {"key": ["five"]}
