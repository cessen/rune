%import std.io
%import std.mem
%import std.error


pub struct DynArray`<T> {
    data: @T,
    size: uint = 0,
    cap: uint = 0,
}
is Dtor


alias self_t: DynArray`<T>


#: Constructor
pub fn new`<T>[cap: uint = 0] -> self_t (
    var self: self_t
    
    if cap > 0 (
        self.data = mem.alloc[cap]
        self.cap = cap
    )
    
    return self
)


#: Destructor
unsafe fn dtor`<T>[self: mut @self_t] (
    # Run destructors on elements if necessary
    if self.size > 0 (
        for i in range[0, self.size] (
            _ = @(self.data + i)
        )
    )
    
    # Free memory if necessary
    if self.cap > 0 (
        mem.free[self.data]
    )
)


#: Index-style access to elements
fn `[]``<T>[self: mut @self_t, i: uint] -> ref mut T (
    if i < self.size (
        return @(self.data + i)
    )
    else (
        throw error.new["Out-of-bounds DynArray access."]
    )
)


#: Reserves enough space for cap elements
pub fn reserve`<T>[self: mut @self_t, cap: uint] (
    if cap > self.cap (
        let new_data = mem.alloc<self_t>[cap]
        mem.copy[new_data, self.data, self.size]
        if self.cap > 0
            mem.free[self.data]
        self.data = new_data
        self.cap = cap
    )
)


#: Pushes an element onto the end of the array
pub fn push`<T>[self: mut @self_t, el: T] (
    if self.cap <= self.size
        self.reserve[self.cap + 1 + (self.cap / 2)]
    
    self.data[self.size] = el
    ++self.size
)


fn main[] (
    var a: DynArray<int> = new[]
    
    a.push[42]
    a.push[13]
    a.push[312]
    
    io.println["{}, {}, {}", a[0], a[1], a[2]]
)
