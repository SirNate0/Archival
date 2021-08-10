# Archival

## Overview

The Archival library is meant to allow serialization and deserialization of C++ objects with minimal boilerplate. Rather than using two separate functions for the to-representation and from-representation tasks that have the same information content in a different form (`archive.Read('x',self.x)` vs `archive.Write('x',self.x)`), we can use a single function that does either task based on whether the archive as a whole was created to serialize or to deserialize the values. E.g. `archive.Serialize('x',self.x)` which will write to the archive the value of x if it's an output backend or try to read from the archive to fill the value of x if it was an input.


### Backend Operations
The Archive class is essentially just a convenience wrapper around the backend. The backend itself still breaks the saving and loading into separate methods, specifically `Get` and `Set`, which take a float for the name and a value to be written. The structures (Series and Groups, discussed below) may be supported by returning another backend instance that will write to the specific sub-structure, though depending on the format (e.g. a flat binary write) simply returning the original backend may be appropriate.

#### Basic Types
The backend must support a small handful of types that can be written or read:
 - bool
 - unsigned/signed char (int8)
 - unsigned/signed short (int16)
 - unsigned/signed int (int32)
 - unsigned/signed long long (int64)
 - float (including NaN, +/-Inf)
 - double (including NaN, +/-Inf)
 - String
 - null (nullptr_t)
 
 These types need not be unique within the backend, but may (not must) be coerced into one another. For example, JSON does not support NaN values, so these could be written as a string `"NaN"` in JSON when a float is passed. If one were to try to read the value as a string, `"NaN"` could be read as a string, and `4.3` may or may not be converted, but is an implementation-defined detail.
 
#### Series
Additionally, the backend must support serializing a series object with a specific name and size. In JSON, this would be an array. The size must be specifically written/read if it is dynamic (i.e. can be different for differnt files) For example:

```
// Vector3
// archive.SerializeSeriesSize('vec',3); <- Size need not be specified if it is constant.
archive.CreateSeriesEntry('vec',v.x);
archive.CreateSeriesEntry('vec',v.y);
archive.CreateSeriesEntry('vec',v.z);


// vector<String>
archive.SerializeSeriesSize('vec',v.size()); // Size must be specified if it is dynamic.
for (auto& x : v)
	archive.CreateSeriesEntry('vec',x);
```

#### Groups
The backend must also support a group structure. In JSON, this would be a table. These are a collection of uniquely named values, which themselves may be basic values, Series, or Groups. This will be the principle usage for complex custom types (simpler types like Vector3 or a Color may prefer using a Series of floats or a string representaiton). The member variable names can be used as the group member names, perhaps converted to a more readable form (e.g. with spaces). Note that the backend may modify the names to conform to backend-specific requirements (e.g. XML node name requirements). These modifications may not preserve similar names, e.g. variations of case, of spacing, and posibly of punctuation/hyphenation, though ideally that would be supported. For dynamic names, for example, a `HashMap<String, Color>` the backend exposes the option to read/write the series names to the archive, which the Archive exposes to the user.

#### Extended Types
Backends may have builtin support for other types beyond the ones listed above. For example, the Urho3D JSON interfaces support a Variant type, and it's XML interface suports VectorN types. Support for custom types is advertisized through virtual functions in the Backend interface, specifically through `virtual bool Supports(const CustomType& /*unused*/) = 0;` interfaces. Note that the parameter MUST actually be unused for correct functioning - it is not possible to conditionally support a CustomType (e.g. supporting a Vector3 iff it is finite) as the condition cannot be known symmetrically in reading and writing. The parameter is simply included so the compiler will select the correct overload. Given the intrusive nature of this change to the Archival library, it is recommended to simply create an `ArchiveValue` implementation that will handle the special case directly. The Backend includes a name by which it might be identified at run-time (in addition to the option of a `dynamic_cast`, so it is possible to specialize behavior for a given type per-backend in the user code.


### Frontend Operation
The `Archive` class is the primary frontend to the Archival library. It must be created with a Backend and a specification of whether the archive is an input (or output) archive. The key method of this class is `Serialize`, which takes a String key for the name and a reference to the value to be (de)serialized. This class defers to the external `Archiver<T>` class template (we use a class to allow partial template specialization), who's `Apply` method tries to do a number of things:

1. If `T` is one of the [Basic Type](#basic-types) it calls `Backend::Get` (or `Backend::Set`).
2. If `T` has an `bool ArchiveValue(Archive& archive, const String& name)` method it calls it.
3. If there exists a `bool ArchiveValue(Archive& archive, const String& name, T& value)` it calls it.
3. A static assert directing the user to specialize the `Archiver` for `T`, as this is how external archive specialization is handled.
