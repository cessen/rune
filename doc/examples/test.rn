#--------------------------------------------------------
# There are three basic kinds of value declarations:

# Constant.  This is a compile-time constant.  The value must be fully
# determineable at compile time, and cannot be modified at run time.
# These are the only kinds of value declarations that can be generic.
# These are also the only kinds of value declarations that can be
# overloaded on type.
const a: int = 5i
const a`<T>: T = foo`<T>[] # A generic version--foo[] must be capable of compile-time evaluation

# Immutable.  This is a run-time constant.  The value can be determined and
# created at run-time, but cannot be modified after creation (except for
# destruction).
let b: int = 5i

# Mutable.  This is like a typical variable from C-style languages.  The
# value can be determined and modified freely at run-time.
var c: int = 5i


#------------------------------------------------------------
# Functions are just another kind of value, and can be
# assigned to constants, immutables, and mutables just like
# any other value.

const func_a = fn [x: int] -> int (return x)
const func_a`<T> = fn [x: T] -> T (return x) # Generic version

let func_b = fn [x: int] -> int (return x)

var func_c = fn [x: int] -> int (return x)

# There is syntactic sugar for declaring compile-time constant functions:
fn func_d[x: int] -> int (return x)
fn func_d`<T>[x: T] -> T (return x) # Generic version


#----------------------------------------------------------
# Scopes can have captures specified to limit the things
# they can access from their parent scopes.

let d = 5i
var e: int

[[d, e]](
    e = a # Error!  Didn't capture a, so cannot access it
    e = d # No problem
)

# You can use this to specify captures on functions:
let func = fn [x: int] -> int [[b, c]](
    return b + c * x
)


#--------------------------------------------------------------
# Function type signatures look almost identical to a function
# literal, but lack a body and parameter names.

# Type of a function that takes no parameters and returns nothing
fn []

# Type of a function that takes one int and returns nothing
fn [int]

# Type of a function that takes two ints and returns an int
fn [int, int] -> int