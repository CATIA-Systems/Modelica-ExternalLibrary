extern crate libc;

use libc::c_char;
use std::ffi::CStr;
use std::{cmp, slice};
use std::fs::read_to_string;


#[no_mangle]
#[allow(non_snake_case)]
pub extern "C" fn externalFunction(
    filename: *const c_char, 
    _moduleName: *const c_char, 
    _functionName: *const c_char, 
    _pythonHome: *const c_char, 
    nu: i32,
    u: *const f64, 
    ny: i32,
    y: *mut f64
) -> *const c_char {

    let filename = unsafe {
        assert!(!filename.is_null());
        CStr::from_ptr(filename).to_str().unwrap()
    };

    let u = unsafe {
        assert!(!u.is_null());
        slice::from_raw_parts(u, nu as usize)
    };

    let y = unsafe {
        assert!(!y.is_null());
        slice::from_raw_parts_mut(y, ny as usize)
    };

    let k = match read_to_string(filename) {
        Ok(text) => match text.trim().parse::<f64>() {
            Ok(value) => value,
            Err(_) => return "Failed to parse value.\0".as_ptr() as *const c_char
        },
        Err(_) => return "Failed to read file.\0".as_ptr() as *const c_char
    };

    for i in 0..cmp::min(nu, ny) as usize {
        y[i] = u[i] + k;
    }

    "\0".as_ptr() as *const c_char
}
