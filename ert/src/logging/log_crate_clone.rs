// REDISTRIBUTED/MODIFIED FROM THE FOLLOWING LICENSE:

// Copyright 2015 The Rust Project Developers. See the COPYRIGHT
// file at the top-level directory of this distribution and at
// http://rust-lang.org/COPYRIGHT.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

use std::fmt;

// import re-used items from the actual crate
use log::{Level, Log, Metadata, Record};
// import our struct implementing the `Log` trait
use super::{ErtLoggable, ErtLogger};

// since we want to log generic data (ie log_struct<T>) a trait object wont work
// we'd need to dynamically look up two methods (Trait::log_struct and then ErtLoggable::type_id)
// so instead we'll make initialization "messier" and give a concrete type
// using... lazy static
// this gives us much less flexibility, but since we control the struct configuration via
// that route is likely better than dynamic dispatch
// lazy static is about as fast as can be, uses std::sync::Once and no other branches inside
// an inline-able Deref impl. The std::sync::Once is a single a branch and atomic Acquire
use lazy_static::lazy_static;

lazy_static! {
    static ref LOGGER: ErtLogger = ErtLogger::new();
}

/// Sets the `log` crate's logger to the ERT logger
/// This allows other libraries, or any code not using or own log macros
/// to log along side our messages.
/// However, those uses will be slower (due to dynamic dispatch) than using
/// the ert macros
pub fn configure_log_crate_logger() -> Result<(), log::SetLoggerError> {
    log::set_logger(logger())
}

pub fn block_on_log_flush() {
    logger().flush()
}

/// Returns a reference to the logger.
///
/// If a logger has not been set, a no-op implementation is returned.
#[inline]
pub fn logger() -> &'static ErtLogger {
    &*LOGGER
}

// WARNING: this is not part of the crate's public API and is subject to change at any time
#[doc(hidden)]
#[inline]
pub fn __private_api_log(
    args: fmt::Arguments,
    level: Level,
    &(target, module_path, file, line): &(&str, &'static str, &'static str, u32),
) {
    logger().log(
        &Record::builder()
            .args(args)
            .level(level)
            .target(target)
            .module_path_static(Some(module_path))
            .file_static(Some(file))
            .line(Some(line))
            .build(),
    );
}

// WARNING: this is not part of the crate's public API and is subject to change at any time
#[doc(hidden)]
#[inline]
pub fn __private_api_log_struct<T: ErtLoggable>(
    level: Level,
    strct: &T,
    &(target, module_path, file, line): &(&str, &'static str, &'static str, u32),
) {
    logger().log_struct(
        strct,
        &Record::builder()
            // default format args (empty string) are used and then ignored by the inner logger
            .level(level)
            .target(target)
            .module_path_static(Some(module_path))
            .file_static(Some(file))
            .line(Some(line))
            .build(),
    );
}

// WARNING: this is not part of the crate's public API and is subject to change at any time
#[doc(hidden)]
#[inline]
pub fn __private_api_enabled(level: Level, target: &str) -> bool {
    logger().enabled(&Metadata::builder().level(level).target(target).build())
}

/// The standard logging macro.
///
/// This macro will generically log with the specified `Level` and `format!`
/// based argument list.
///
/// # Examples
///
/// ```edition2018
/// use log::{log, Level};
///
/// # fn main() {
/// let data = (42, "Forty-two");
/// let private_data = "private";
///
/// log!(Level::Error, "Received errors: {}, {}", data.0, data.1);
/// log!(target: "app_events", Level::Warn, "App warning: {}, {}, {}",
///     data.0, data.1, private_data);
/// # }
/// ```
#[macro_export]
macro_rules! log {
    (target: $target:expr, $lvl:expr, $($arg:tt)+) => ({
        let lvl = $lvl;
        if lvl <= $crate::log_rexp::STATIC_MAX_LEVEL && lvl <= $crate::log_rexp::max_level() {
            $crate::log_rexp::__private_api_log(
                format_args!($($arg)+),
                lvl,
                &($target, module_path!(), file!(), line!()),
            );
        }
    });
    ($lvl:expr, $($arg:tt)+) => (crate::log!(target: module_path!(), $lvl, $($arg)+))
}

/// Logs a message at the error level.
///
/// # Examples
///
/// ```edition2018
/// use log::error;
///
/// # fn main() {
/// let (err_info, port) = ("No connection", 22);
///
/// error!("Error: {} on port {}", err_info, port);
/// error!(target: "app_events", "App Error: {}, Port: {}", err_info, 22);
/// # }
/// ```
#[macro_export]
macro_rules! error {
    (target: $target:expr, $($arg:tt)+) => (
        crate::log!(target: $target, $crate::log_rexp::Level::Error, $($arg)+);
    );
    ($($arg:tt)+) => (
        crate::log!($crate::log_rexp::Level::Error, $($arg)+);
    )
}

/// Logs a message at the warn level.
#[macro_export]
macro_rules! warn {
    (target: $target:expr, $($arg:tt)+) => (
        crate::log!(target: $target, $crate::log_rexp::Level::Warn, $($arg)+);
    );
    ($($arg:tt)+) => (
        crate::log!($crate::log_rexp::Level::Warn, $($arg)+);
    )
}

/// Logs a message at the info level.
#[macro_export]
macro_rules! info {
    (target: $target:expr, $($arg:tt)+) => (
        crate::log!(target: $target, $crate::log_rexp::Level::Info, $($arg)+);
    );
    ($($arg:tt)+) => (
        crate::log!($crate::log_rexp::Level::Info, $($arg)+);
    )
}

/// Logs a message at the debug level.
#[macro_export]
macro_rules! debug {
    (target: $target:expr, $($arg:tt)+) => (
        crate::log!(target: $target, $crate::log_rexp::Level::Debug, $($arg)+);
    );
    ($($arg:tt)+) => (
        crate::log!($crate::log_rexp::Level::Debug, $($arg)+);
    )
}

/// Logs a message at the trace level.
#[macro_export]
macro_rules! trace {
    (target: $target:expr, $($arg:tt)+) => (
        crate::log!(target: $target, $crate::log_rexp::Level::Trace, $($arg)+);
    );
    ($($arg:tt)+) => (
        crate::log!($crate::log_rexp::Level::Trace, $($arg)+);
    )
}

/*
================
STRUCT LOGGING
================
*/

/// The standard struct logging macro.
///
/// This macro will generically log with the specified `Level`, and `ErtLoggable` struct.
///
/// # Examples
///
/// ```edition2018
/// TODO
/// ```
#[macro_export]
macro_rules! log_struct {
    (target: $target:expr, $lvl:expr, $strct:expr) => {{
        let lvl = $lvl;
        if lvl <= $crate::log_rexp::STATIC_MAX_LEVEL && lvl <= $crate::log_rexp::max_level() {
            $crate::log_rexp::__private_api_log_struct(
                lvl,
                $strct,
                &($target, module_path!(), file!(), line!()),
            );
        }
    }};
    ($lvl:expr, $strct:expr) => {
        crate::log_struct!(target: module_path!(), $lvl, $strct)
    };
}

/// Logs a struct at the error level.
///
/// # Examples
///
/// ```edition2018
/// TODO
/// ```
#[macro_export]
macro_rules! error_struct {
    (target: $target:expr, $strct:expr) => {
        crate::log_struct!(target: $target, $crate::log_rexp::Level::Error, $strct);
    };
    ($strct:expr) => {
        crate::log_struct!($crate::log_rexp::Level::Error, $strct);
    };
}

/// Logs a struct at the warn level.
#[macro_export]
macro_rules! warn_struct {
    (target: $target:expr, $strct:expr) => {
        crate::log_struct!(target: $target, $crate::log_rexp::Level::Warn, $strct);
    };
    ($strct:expr) => {
        crate::log_struct!($crate::log_rexp::Level::Warn, $strct);
    };
}

/// Logs a struct at the info level.
#[macro_export]
macro_rules! info_struct {
    (target: $target:expr, $strct:expr) => {
        crate::log_struct!(target: $target, $crate::log_rexp::Level::Info, $strct);
    };
    ($strct:expr) => {
        crate::log_struct!($crate::log_rexp::Level::Info, $strct);
    };
}

/// Logs a struct at the debug level.
#[macro_export]
macro_rules! debug_struct {
    (target: $target:expr, $strct:expr) => {
        crate::log_struct!(target: $target, $crate::log_rexp::Level::Debug, $strct);
    };
    ($strct:expr) => {
        crate::log_struct!($crate::log_rexp::Level::Debug, $strct);
    };
}

/// Logs a struct at the trace level.
#[macro_export]
macro_rules! trace_struct {
    (target: $target:expr, $strct:expr) => {
        crate::log_struct!(target: $target, $crate::log_rexp::Level::Trace, $strct);
    };
    ($strct:expr) => {
        crate::log_struct!($crate::log_rexp::Level::Trace, $strct);
    };
}
