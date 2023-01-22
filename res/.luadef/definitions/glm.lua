---@meta 

--- Known issue: You must right-multiply vectors by numbers, otherwise the lua language server 
--- will not understand it and think it's the result is a number!
--- Will be fixed eventually by sumneko as the issue is open on the github repo
--- For now, you can add ---@cast wathever glm.vec3 next to the line to fix the warnings

---@class glm
---@field pi number 
---@field half_pi number
---@field two_pi number
local glm = {}

---@class glm.vec2
---@operator add(glm.vec2): glm.vec2 Adds the vectors component-wise
---@operator sub(glm.vec2): glm.vec2 Substracts the vectors component-wise
---@operator mul(glm.vec2): glm.vec2 Multiplies the vectors component-wise
---@operator mul(number): glm.vec2 Multiplies every component by the number
---@operator div(glm.vec2): glm.vec2 Divides the vectors component-wise
---@operator div(number): glm.vec2 Divides every component by the number
---@operator unm(): glm.vec2 Obtains the opposite vector
---@field x number
---@field y number
glm.vec2 = {}
---@return glm.vec2
---@param param1? number|glm.vec3|glm.vec4 If number, initialize as (x, y). Otherwise, initialize from higher dimension vector by trimming
---@param param2? number Only present if initializing as (x, y)
function glm.vec2.new(param1, param2) end
---@return number x x component
---@return number y y component 
--- Unpacks the vector, useful to pass to functons which take individual numbers
--- To pack a vector, simply call new() with the numbers.
function glm.vec2:unpack() end


---@class glm.vec3
---@operator add(glm.vec3): glm.vec3 Adds the vectors component-wise
---@operator sub(glm.vec3): glm.vec3 Substracts the vectors component-wise
---@operator mul(glm.vec3): glm.vec3 Multiplies the vectors component-wise
---@operator mul(glm.quat): glm.vec3 Rotates the vector using the inverse quaternion. You may want to use the reverse order!
---@operator mul(number): glm.vec3 Multiplies every component by the number
---@operator mul(glm.mat3): glm.vec3 Left-multiplies the vector with the 3x3 matrix (you may want right-multiplication!)
---@operator div(glm.vec3): glm.vec3 Divides the vectors component-wise
---@operator div(glm.mat3): glm.vec3 Left-divides the vector with the 3x3 matrix
---@operator div(number): glm.vec3 Divides every component by the number
---@operator unm(): glm.vec3 Obtains the opposite vector
---@field x number
---@field y number
---@field z number
glm.vec3 = {}
---@return glm.vec3
---@param param1? number|glm.vec4|glm.vec2
---@param param2? number Only present if initializing as (x, y, z) or (vec2, z)
---@param param3? number Only present if initializing as (x, y, z)
function glm.vec3.new(param1, param2, param3) end
---@return number x x component
---@return number y y component 
---@return number z z component
--- Unpacks the vector, useful to pass to functons which take individual numbers
--- To pack a vector, simply call new() with the numbers.
function glm.vec3:unpack() end

---@return glm.vec2 vector The vector formed by the x and y components 
function glm.vec3:to_vec2() end

---@class glm.vec4
---@operator add(glm.vec4): glm.vec4 Adds the vectors component-wise
---@operator sub(glm.vec4): glm.vec4 Substracts the vectors component-wise
---@operator mul(glm.vec4): glm.vec4 Multiplies the vectors component-wise
---@operator mul(glm.mat4): glm.vec4 Left-multiplies the vector with the 4x4 matrix (you may want right-multiplication!)
---@operator mul(glm.quat): glm.vec4 Rotates the vector using the inverse quaternion. You may want to use the reverse order!
---@operator mul(number): glm.vec4 Multiplies every component by the number
---@operator div(glm.vec4): glm.vec4 Divides the vectors component-wise
---@operator div(number): glm.vec4 Divides every component by the number
---@operator div(glm.mat4): glm.vec4 Left-divides the vector with the 4x4 matrix
---@operator unm(): glm.vec4 Obtains the opposite vector
---@field x number
---@field y number
---@field z number
---@field w number
glm.vec4 = {}

---@return glm.vec4
---@param param1? number|glm.vec3
---@param param2? number Only present if initializing as (x, y, z, w) or (vec3, w)
---@param param3? number Only present if initializing as (x, y, z, w)
---@param param4? number Only present if initializing as (x, y, z, w)
function glm.vec4.new(param1, param2, param3, param4) end

---@return number x x component
---@return number y y component 
---@return number z z component
---@return number w w component
--- Unpacks the vector, useful to pass to functons which take individual numbers
--- To pack a vector, simply call new() with the numbers.
function glm.vec4:unpack() end

---@return glm.vec2 vector The vector formed by the x and y components 
function glm.vec4:to_vec2() end

---@return glm.vec3 vector The vector formed by the x, y and z components 
function glm.vec4:to_vec3() end


---@class glm.mat3
---@operator add(glm.mat3): glm.mat3 Adds the matrices component-wise
---@operator sub(glm.mat3): glm.mat3 Substracts the matrices component-wise
---@operator add(number): glm.mat3 Adds the number from all elements of the matrix
---@operator sub(number): glm.mat3 Substracts the number from all elements of the matrix
---@operator mul(number): glm.mat3 Multiplies the number by all elements of the matrix
---@operator mul(glm.mat3): glm.mat3 Performs matrix multiplication, OpenGL style
---@operator mul(glm.vec3): glm.vec3 Left-multiplies the vector with the matrix. This is what you typically use.
---@operator div(number): glm.mat3  Divides all elements of the matrix by the number
---@operator div(glm.mat3): glm.mat3 Performs matrix division (multiply by the inverse), OpenGL style
---@operator div(glm.vec3): glm.vec3 Left-divides the vector with the matrix. This is what you typically use.
glm.mat3 = {}

---@return glm.mat3
---@param param1? number|glm.vec3
---@param param2? glm.vec3 Only present if initializing as (vec3, vec3, vec3)
---@param param3? glm.vec3 Only present if initializing as (vec3, vec3, vec3)
function glm.mat3.new(param1, param2, param3) end

---@class glm.mat4
---@operator add(glm.mat4): glm.mat4 Adds the matrices component-wise
---@operator sub(glm.mat4): glm.mat4 Substracts the matrices component-wise
---@operator add(number): glm.mat4 Adds the number from all elements of the matrix
---@operator sub(number): glm.mat4 Substracts the number from all elements of the matrix
---@operator mul(number): glm.mat4 Multiplies the number by all elements of the matrix
---@operator mul(glm.mat4): glm.mat4 Performs matrix multiplication, OpenGL style
---@operator mul(glm.vec4): glm.vec4 Left-multiplies the vector with the matrix. This is what you typically use.
---@operator div(number): glm.mat4  Divides all elements of the matrix by the number
---@operator div(glm.mat4): glm.mat4 Performs matrix division (multiply by the inverse), OpenGL style
---@operator div(glm.vec4): glm.vec4 Left-divides the vector with the matrix. This is what you typically use.
glm.mat4 = {}

---@return glm.mat4
---@param param1? number|glm.vec4
---@param param2? glm.vec4 Only present if initializing as (vec4, vec4, vec4, vec4)
---@param param3? glm.vec4 Only present if initializing as (vec4, vec4, vec4, vec4)
---@param param4? glm.vec4 Only present if initializing as (vec4, vec4, vec4, vec4)
function glm.mat4.new(param1, param2, param3, param4) end


---@class glm.quat
---@operator mul(glm.quat): glm.quat Multiplies the quaternions
---@operator mul(glm.vec3): glm.vec3 Rotates the vector using the quaternion. Intended order
---@operator mul(glm.vec4): glm.vec4 Rotates the vector using the quaternion. Intended order
---@operator unm(): glm.quat Obtains the opposite vector
---@field x number
---@field y number
---@field z number
---@field w number
glm.quat = {}

---@return glm.quat
---@param param1? number|glm.vec3|glm.mat3|glm.mat4
---@param param2? number|glm.vec3
---@param param3? number
---@param param4? number
--- Creates a quaternion. If given a mat3 or mat4 it extracts the rotation components
--- Valid constructor forms:
---    new(w, x, y, z)
---    new(w, vec3)
---    new(mat3)
---    new(mat4)
function glm.quat.new(param1, param2, param3, param4) end

---@return number w w component
---@return number x x component 
---@return number y y component
---@return number z z component
--- Unpacks the quaternion, useful to pass to functons which take individual numbers.
--- Careful with order, w goes first!
--- To pack a quaternion, simply call new() with the numbers.
function glm.quat:unpack() end


---------------------------------------------
-- Core functions 
---------------------------------------------
-- TODO: Overloads dont appear to be too well understood by the language server?

---@return number|glm.vec2|glm.vec3|glm.vec4
---@param v number|glm.vec2|glm.vec3|glm.vec4
--- Returns the component-wise absolute value of a vector, or a number
function glm.abs(v) end

---@return number|glm.vec2|glm.vec3|glm.vec4
---@param v number|glm.vec2|glm.vec3|glm.vec4
--- Returns the component-wise ceiled value of a vector, or a number
function glm.ceil(v) end

---@return number|glm.vec2|glm.vec3|glm.vec4
---@param v number|glm.vec2|glm.vec3|glm.vec4
--- Returns the component-wise floored value of a vector, or a number
function glm.floor(v) end

---@return number|glm.vec2|glm.vec3|glm.vec4
---@param v number|glm.vec2|glm.vec3|glm.vec4
--- Returns the component-wise rounded value of a vector, or a number
function glm.round(v) end

---@return number|glm.vec2|glm.vec3|glm.vec4
---@param v number|glm.vec2|glm.vec3|glm.vec4
---@param lower number|glm.vec2|glm.vec3|glm.vec4
---@param upper number|glm.vec2|glm.vec3|glm.vec4
--- If lower and upper are numbers, v may be any of the types. Each component in v will be clamped according to the scalars upper and lower 
--- If lower and upper are vectors of the same dimension, v must be of the same dimension. Each component in v will be clamped with the components of upper and lower
function glm.clamp(v, lower, upper) end

---@return number|glm.vec2|glm.vec3|glm.vec4
---@param v number|glm.vec2|glm.vec3|glm.vec4
--- Returns the component-wise fractional part value of a vector, or a number
function glm.fract(v) end

---@return boolean
---@param n number
--- Is the number infinite?
function glm.isinf(n) end

---@return boolean
---@param n number
--- Is the number NaN?
function glm.isnan(n) end

---@return number
---@param a number
---@param b number
---@overload fun(a: number, b: number): number
---@overload fun(a: glm.vec2, b: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3, b: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4, b: glm.vec4): glm.vec4
---@overload fun(a: glm.vec2, b: number): glm.vec4
---@overload fun(a: glm.vec3, b: number): glm.vec4
---@overload fun(a: glm.vec4, b: number): glm.vec4
--- If a and b are both vectors or numbers, max is performed component wise
--- If a is a vector and b a number, each component in a is now max(a, number)
---@nodiscard
function glm.max(a, b) end

---@return number
---@param a number
---@param b number
---@overload fun(a: number, b: number): number
---@overload fun(a: glm.vec2, b: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3, b: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4, b: glm.vec4): glm.vec4
---@overload fun(a: glm.vec2, b: number): glm.vec4
---@overload fun(a: glm.vec3, b: number): glm.vec4
---@overload fun(a: glm.vec4, b: number): glm.vec4
--- If a and b are both vectors or numbers, min is performed component wise
--- If a is a vector and b a number, each component in a is now min(a, number)
---@nodiscard
function glm.min(a, b) end

---@return number
---@param a number
---@param b number
---@param n number
---@overload fun(a: number, b: number, n:number): number
---@overload fun(a: glm.vec2, b: glm.vec2, n:number): glm.vec2
---@overload fun(a: glm.vec3, b: glm.vec3, n:number): glm.vec3
---@overload fun(a: glm.vec4, b: glm.vec4, n:number): glm.vec4
---@overload fun(a: glm.quat, b: glm.quat, n:number): glm.quat
--- Linear interpolation between two values (vector, number or quaternion) using a number n
---@nodiscard
function glm.mix(a, b, n) end

---@return number
---@param a number
---@param b number
---@overload fun(a: number, b: number): number
---@overload fun(a: glm.vec2, b: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3, b: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4, b: glm.vec4): glm.vec4
--- Component wise mod operation
---@nodiscard
function glm.mod(a, b) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
--- Component wise sign operation
---@nodiscard
function glm.sign(a) end

---@return number
---@param a number
---@param b number
---@param n number
---@overload fun(a: number, b: number, n:number): number
---@overload fun(a: glm.vec2, b: glm.vec2, n:number): glm.vec2
---@overload fun(a: glm.vec3, b: glm.vec3, n:number): glm.vec3
---@overload fun(a: glm.vec4, b: glm.vec4, n:number): glm.vec4
---@overload fun(a: glm.quat, b: glm.quat, n:number): glm.quat
--- Hermite interpolation between two values (vector, number or quaternion) using a number n
---@nodiscard
function glm.smoothstep(a, b, n) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
--- Component wise trunc operation
---@nodiscard
function glm.trunc(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
--- Component wise exp operation
---@nodiscard
function glm.exp(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
--- Component wise exp2 (power of two) operation
---@nodiscard
function glm.exp2(a) end


---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
--- Component 1.0 / sqrt(x) operation. Faster than doing it yourself
---@nodiscard
function glm.inversesqrt(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
--- Component log operation. Faster than doing it yourself
---@nodiscard
function glm.log(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
--- Component log2 operation. Faster than doing it yourself
---@nodiscard
function glm.log2(a) end

---@return number
---@param a number
---@param b number
---@overload fun(a: number, b: number): number
---@overload fun(a: glm.vec2, b: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3, b: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4, b: glm.vec4): glm.vec4
--- Component wise pow operation
---@nodiscard
function glm.pow(a, b) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
--- Component sqrt(x) operation. Faster than doing it yourself
---@nodiscard
function glm.sqrt(a) end

---@return glm.quat
---@param quat glm.quat
--- Computes the quaternion conjugate
function glm.conjugate(quat) end

---@return number
---@param a glm.vec3
---@param b glm.vec3
---@overload fun(a: glm.vec2, b: glm.vec2): number
---@overload fun(a: glm.vec3, b: glm.vec3): number
---@overload fun(a: glm.vec4, b: glm.vec4): number
--- Distance between two vectors
---@nodiscard
function glm.distance(a, b) end

---@return number
---@param a glm.vec3
---@param b glm.vec3
---@overload fun(a: glm.vec2, b: glm.vec2): number
---@overload fun(a: glm.vec3, b: glm.vec3): number
---@overload fun(a: glm.vec4, b: glm.vec4): number
---@overload fun(a: glm.quat, b: glm.quat): number
--- Dot product between two vectors / quaternions
---@nodiscard
function glm.dot(a, b) end

---@return number
---@param a glm.vec3
---@overload fun(a: glm.vec2): number
---@overload fun(a: glm.vec3): number
---@overload fun(a: glm.vec4): number
--- Computes length of vector
---@nodiscard
function glm.length(a) end

---@param a glm.vec3
---@return glm.vec3
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.quat): glm.quat
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec4): glm.vec4
--- Normalizes a vector / quaternion
---@nodiscard
function glm.normalize(a) end


---@return glm.vec3
---@param a glm.vec3 incident vector
---@param N glm.vec3 normal vector
---@overload fun(a: glm.vec2, N: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3, N: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4, N: glm.vec4): glm.vec4
--- Returns the result of reflecting a in plane formed by normal vector N
---@nodiscard
function glm.reflect(a, N) end

---@return glm.vec3
---@param a glm.vec3 incident vector
---@param N glm.vec3 normal vector
---@param eta number ratio of indices of refraction
---@overload fun(a: glm.vec2, N: glm.vec2, eta: number): glm.vec2
---@overload fun(a: glm.vec3, N: glm.vec3, eta: number): glm.vec3
---@overload fun(a: glm.vec4, N: glm.vec4, eta: number): glm.vec4
--- Returns the result of refracting a in plane formed by normal vector N with ratio of IOR eta
---@nodiscard
function glm.refract(a, N, eta) end


---@return number
---@param m glm.mat3
---@overload fun(m: glm.mat4): number
function glm.determinant(m) end

---@return glm.mat3 
---@param m glm.mat3
---@overload fun(m: glm.mat4): glm.mat4
---@overload fun(m: glm.quat): glm.quat
function glm.inverse(m) end

---@return glm.mat3 
---@param m1 glm.mat3
---@param m2 glm.mat3
---@overload fun(m1: glm.mat4, m2: glm.mat4): glm.mat4
--- Performs component wise multiplication of matrix elements
function glm.matrix_comp_mult(m1, m2) end

---@return glm.mat3 
---@param m glm.mat3
---@overload fun(m: glm.mat4): glm.mat4
function glm.transpose(m) end


---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.acos(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.acosh(a) end


---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.asin(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.asinh(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.atan(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.atanh(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.cos(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.cosh(a) end


---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.degrees(a) end

---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.radians(a) end


---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.sin(a) end


---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.sinh(a) end


---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.tan(a) end


---@return number
---@param a number
---@overload fun(a: number): number
---@overload fun(a: glm.vec2): glm.vec2
---@overload fun(a: glm.vec3): glm.vec3
---@overload fun(a: glm.vec4): glm.vec4
---@nodiscard
function glm.tanh(a) end


---@return glm.mat4
---@param left number
---@param right number
---@param bottom number
---@param top number
---@param near number
---@param far number
---@nodiscard
function glm.frustum(left, right, bottom, top, near, far) end

---@return glm.mat4
---@param fovy number
---@param aspect number
---@param near number
---@nodiscard
function glm.infinite_perspective(fovy, aspect, near) end


---@return glm.mat4
---@param eye glm.vec3
---@param center glm.vec3
---@param up glm.vec3
---@nodiscard
function glm.look_at(eye, center, up) end


---@return glm.mat4
---@param left number
---@param right number
---@param bottom number
---@param top number
---@param zNear? number
---@param zFar? number
---@nodiscard
function glm.ortho(left, right, bottom, top, zNear, zFar) end

---@return glm.mat4
---@param fovy number
---@param aspect number
---@param near number
---@param far number
---@nodiscard
function glm.perspective(fovy, aspect, near, far) end

---@return glm.mat4
---@param fov number
---@param width number
---@param height number
---@param near number
---@param far number
---@nodiscard
function glm.perspective_fov(fov, width, height, near, far) end

---@return glm.mat4
---@param m glm.mat4 Matrix to translate
---@param v glm.vec3 Translation vector
function glm.translate(m, v) end

---@return glm.mat4
---@param m glm.mat4 Matrix to scale
---@param v glm.vec3 Scale vector
function glm.scale(m, v) end



---@return glm.mat4
---@param param1 glm.mat4
---@param param2 number
---@param param3 glm.vec3
---@overload fun(param1: glm.quat, param2: glm.vec3): glm.vec3
---@nodiscard
--- Either does a matrix rotation by angle axis if called like rotate(matrix, angle, axis) 
--- or a vector quaternion rotation if called like rotate(quat, vector)
function glm.rotate(param1, param2, param3) end


----------------------------------
-- Util functions from MathUtil
-- Not really part of glm!
---------------------------------

---@param cubic glm.vec3 normalized cube coordinates
---@return glm.vec3 normalized spherical coordinates
--- Used by the planetary system to "move" vertices from cube space to sphere space
function glm.cube_to_sphere(cubic) end

---@param sphere glm.vec3 normalized spherical coordinates
---@return glm.vec3 normalized cube coordinates
--- Used by the planetary system to "move" vertices from sphere space to cube space
function glm.sphere_to_cube(sphere) end

---@param from glm.vec3 Origin vector
---@param to glm.vec3 Destination vector
---@return glm.mat4 Rotation matrix that rotates from to to
function glm.rotate_from_to(from, to) end

---@param azimuth number
---@param altitude number
---@param radius number
---@return glm.vec3 3D vector that represents the spherical coordinates. Pole vector is y (0, 1, 0)
function glm.spherical_to_euclidean(azimuth, altitude, radius) end

---@param vec glm.vec3 Euclidean coordinates, pole must be y-up (0, 1, 0)
---@return number azimuth
---@return number altitude
---@return number radius
function glm.euclidean_to_spherical(vec) end

---@param azimuth number
---@param altitude number
---@return glm.vec3 3D vector that represents the spherical coordinates. Pole vector is y (0, 1, 0)
--- Assumes radius 1, slightly faster
function glm.spherical_to_euclidean_r1(azimuth, altitude) end

---@param vec glm.vec3 Euclidean coordinates, pole must be y-up (0, 1, 0). Must be normalized
---@return number azimuth
---@return number altitude
--- Assumes radius to be 1 in vec!
function glm.euclidean_to_spherical_r1(vec) end

---@param from glm.vec3 Eye position
---@param to glm.vec3 What to look at
---@param up glm.vec3 Up vector to use
---@param alt_up glm.vec3 Alternate up vector to use if we are looking in up direction
---@return glm.quat
function glm.quat_look_at(from, to, up, alt_up) end

---@param line_a glm.vec3
---@param line_b glm.vec3
---@param from glm.vec3
---@return number Distance of point from to line that goes from line_a to line_b
function glm.distance_to_line(line_a, line_b, from) end

return glm

