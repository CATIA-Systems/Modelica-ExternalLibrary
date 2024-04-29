use libc::c_char;
use std::ffi::CStr;
use std::ffi::CString;
use std::env;


#[link(name = "ExternalLibrary.dll", kind="dylib")]
extern { 
    fn externalFunction(
        filename: *const c_char, 
        _moduleName: *const c_char, 
        _functionName: *const c_char, 
        _pythonHome: *const c_char, 
        nu: i32,
        u: *const f64,
        ny: i32,
        y: *mut f64
    ) -> *const c_char;
}

fn main() {

    let args: Vec<String> = env::args().collect();

    if args.len() != 2 {
        println!("Usage: external_library_runner <filename>");
        return;
    }

    let arg1 = args.get(1).unwrap();

    let filename = CString::new(arg1.as_str()).unwrap();

    let empty_string = CStr::from_bytes_with_nul(b"\0").unwrap();
    let u = [1.0, 2.0, 3.0];
    let y = &mut [0.0; 3];

    let c_str;

    unsafe {
        let message = externalFunction(filename.as_ptr(), empty_string.as_ptr(), empty_string.as_ptr(), empty_string.as_ptr(), 3, u.as_ptr(), 3, y.as_mut_ptr());
        c_str = CStr::from_ptr(message);
    }

    println!("message is {:?}", c_str);
    println!("y is {:?}", y);
}
