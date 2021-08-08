# Archival Project

But first, some good news.

## Good News

### God created us to be with Him.
 - And in the beginning, we were, and everything was very good. However:

### Our sin separates us from God. 
 - Sin is basically another word for wrongdoing: lying, cheating, stealing, murder, lust, greed, etc.
 
### Sins cannot be removed by good deeds. 
 - Because God is perfect, there must be just punishment for sin.
 
### Paying the price for our sin, Jesus died and rose again.
 - To allow us to be with Him again, God became man (Jesus), lived the perfect life that we have not lived, and then, though he was not guilty himself, he took the penalty for our sin on himself so that he might show mercy to us.
 
### Everyone who trusts in Him alone has eternal life.
 - "For God so loved the world that he gave his one and only Son \[Jesus\], that whoever believes in him shall not perish but have eternal life. For God did not send his Son into the world to condemn the world, but to save the world through him. Whoever believes in him is not condemned, but whoever does not believe stands condemned already because they have not believed in the name of God’s one and only Son."
 
### Life with Jesus starts now and lasts forever.

 - So, if you have not yet made your relationship with God right by  trusting in Jesus, I would invite you to do so now. Admit to yourself and to God that you have sinned, that you have separated yourself from God and deserve his punishment for that sin, and trust in Jesus's offer of salvation (an offer based on his perfect life, his death on the cross, and his resurrection afterwards). Commit to following Jesus, trusting in him to save you.

"For all have sinned and fall short of the glory of God." And "the wages of sin is death, but the gift of God is eternal life in Christ Jesus our Lord." If you have any questions on the matter, feel free to reach out to me or to a local Church.

## About the Project

I was unsatisfied with the current serialization code for the Urho3D library, so I decided to create a different method for it based off of an implement one myself. The old model has a separate method of serialization for so-called `Resource`s (which are meant to be loaded once and used multiple times) and `Serializable`s (which are things like game objects, that give you one object in code per object serialized). While a `Serializable` can be declared with relatively little effort using a few macro classes, and provides a bit of reflection and an automated way of both saving and loading (provided you don't want to do anything too complex), the resulting files are rather ugly and would be difficult to create by hand. On the other hand, a `Resource` produces much nicer files, but at the price of having to write separate loading and saving routines (which inevitably end up feeling very repetitive, as to load you say that `x = src.ReadInt()` and to save you call `dest.Write(x)`. I wanted a system that would give the best of both worlds, succinct files that would be easy to create by hand, with support for multiple backend formats (XML, JSON, binary, etc.). 

The fundamental approach taken by the project is to note that the `Resource` approach can be encapsulated in a single function:
```cpp
void ArchiveValue(Archive& ar, T& x)
{
    if (ar.IsInput())
        x = ar.Read<T>();
    else
        ar.Write<T>(x);
}
```

We can further exploit a few C++ features to make this process easier on the user:
 
 - The backend will support reading/writing only a handful of predetermined types (basically, the primitive C++ types, strings, and a few others.
 - The frontend wraps that support into a single function as above `ArchiveValue`.
 - The user then implements the `ArchiveValue` for their type, and thanks to C++ templates and overload resolution, the function will be called whenever someone tries to archive an instance of that type.
 
We must now think about what the Archive backend should do. Fundamentally, I think the JSON-structure is a good way to hold most data. All of the primitive data (numbers and strings and such) in the structure is either labeled by an index (arrays or lists) or by a string (tables or dictionaries) in the containing structure. All of the data that is labeled by the strings/indices is then either a primitive data type or another set of data. Because string labels are assumed to be the dominant form for backends and usability (as strings convey meaning, but property #1 could refer to anything), every entry to the Archive must be passed a string to be used as the name. As such, the signature of our function is actually `void ArchiveValue(Archive& ar, const String& name, T& x)`

To model the dictionaires and lists there are two main grouping mechanisms for the Archive: Groups and Series. A Group is basically the equivalent to a JSON table, and is the principle backend. Most user types will be represented as a Group, unless the type is basically a wrapper for a primitive type ~~or the type is some sort of Array or Vector structure~~. A series entry is basically just a second entry that should have the same name as another element but be stored separately. It's primary use is for class memebers that are list-like.

To simplify the handling of the Archival process and to allow more human-readable data structures, a sentinel name is reserved by the Archive backend to refer to inline elements. By default it will be "value", but it can be changed by the user at run-time to allow for "value" to be written as the label for compatibility with other programs (mainly Urho3D). Thus, if we open a group `'makeInline'` and request it archive the value `42` with our inline sentinel, we will obtain `'makeInline' : 42` instead of `'makeInline' : {'value' : 42}`. There are two primary uses of this feature:
 1. A calling class can 'promote' a member to be at it's own level and serielize it's values in the same dictionary as the class.
 2. Series entries by default create tables as the elements in the arrays. Inline elements allow for `[1,2,3]` instead of `[{'value':1},{'value':2},{'value':3}]`.
 
Note that the backend is not required to handle the inline sentinel in any special way - for example, in XML it may not be possible to convert a series to a list due to naming restrictions of XML nodes.

## Installation
TODO

## Contract (Assumptions and Rules)

There are some (soft) rules for how the program can be used to ensure compatibility with a broad number of backends. A lot of them are not enforced by the code, but you may have broken results if you don't follow them (perhaps only on one or two backends).

 - Data must always be written and read in the same order. While lables are the principle method supported, the backend is not required to do anything with them, and may be based solely on the ordering of the written values (e.g. a binary backend).
    - To overcome the limitations this imposes on dynamic data, two methods are provided - `SerializeSeriesSize` and `SerializeEntryNames`, which will archive the size of a list and archive the keys of a dictionary, respectively. The principle use of the former is for dynamic list-like elements (and the latter follows a similar pattern, though there may be greater complexity in getting or storing the keys):
    
        ```cpp
    
        // You want to serialize a Vector<int> v into archive ar with label "data".
        
        // This will store the size of v on write (if needed) and resize v to fit the number of elements on read.
        ar.SerializeSeriesSize("data",v);
        
        // On write this is obviously good. On read we have just resized v so that it will also work.
        for (auto& x: v)
            // Actually write the value to the output on write, or load the value to x on read.
            ar.CreateSeriesEntry("data").SerializeInline(x);
        ```
    - To avoid issues, especially with a binary backend, always Serialize all of the series/group entries that were present for the first call. Do not `SerializeSeriesSize("data",3)` and then create only one `"data"` entry.
    - To make it possible to skip unnecessary values in the archiving, Archive provides a `WriteConditional` method. This returns a 'magic' class with methods that allow writing a value if the condition was true (`Then`) or if it was false (`Else`). On human-friendly formats like JSON, this allows you to skip unnecessary values (like default values) when writing to the file, but still check for the values when reading it. It also permits the binary backend to avoid a potentially large write in exchange for a single bool write. At present, these conditionals are unlabeled, but it is likely that the API will require them to be labeled in the future. As an example
        ```cpp
        if (!ar.WriteConditional(v == GetDefault()).Then("v", v))
        {
            // We may have failed to read the value (we should not have failed to write it).
            v = GetDefault();
        }
        
        ```
 - Overlapping value/group names should be avoided because of JSON. While in XML you can repeat a node name as many times as you'd like, in JSON you cannot have `{'a':1,'a':2}`. This applies both when reading and writing files, and also applies to promoted inline elements - it is your responsibility to ensure that the promoted member does not clash with another member.
 - While it may never come into play, assume that the backend may be case insensitive, such that 'Abc" and 'ABC and 'abc' would count as having overlapping names, but that it may not be as well, such that reading 'ABC' from a data written as 'abc' is not gauranteed to work (and likely wouldn't).
 - Do not start names with a number, also for XML compatibility.
 - Spaces in names should be avoided for compatibility with XML. Internally they will have to be converted to a different character (`_` or `.` most probably), so loading and saving should be consistent, but "red_dog", "red dog", "reddog", and "red.dog" cannot all be unique names in XML. 
    - Names starting with XML should be avoided for the same reason, though internally they will probably be converted to '_XML' and not fail to be written or read back.
 - Periods should be avoided in general for the sake of more readable TOML, so that you don't need `Zoo.'red.dog'` to represent the key.
 
 
## Backends
I've talked about a number of backends, both planned and implemented.

 - JSON: implemented
 - XML: planned
 - Binary: planned
 - NoOp: implemented (simply fails to write anything, used as a backend for failed conditionals).
 - ImGui: implemented
 
### Backend Hints
TODO: comment on them.
 
## Structure
TODO: improve it so the actual Archival source is more separate from the example.

Important files:

 - Archive.h - includes the frontend code to serialize values.
 - ArchiveDetail.h - defines the principle backends and some template magic.
 - ArchiveDetail.cpp - implementations for the backends.
 
Important classes:

 - Archive - the frontend class. Handles the calls to serialize values, provides convenience functions for the Inline values, etc.
 - Backend - base class for all Archive backends.
 - ___Backend - Implements a given backend.
 
Important Functions:
 - TODO
 
 ---
Scripture quotations taken from The Holy Bible, New International Version® NIV®  
Copyright © 1973 1978 1984 2011 by Biblica, Inc. TM  
Used by permission. All rights reserved worldwide.


`virtualGizmo3D` for the ImGui is included in source form, with license and readme in the folder.

`imgui_UrhoString.*` is based on the imgui std::string implenentation, and heavily derived from the `imgui_stdlib.*` files.

The `CMake/` files are copied from the Urho3D project, as well as the MultipleViewports sample to use for the example.
