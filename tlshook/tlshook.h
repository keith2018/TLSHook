/*
 * TLSHook
 * @author 	: keith@robot9.me
 *
 */
#pragma once

namespace TLSHook {

/**
 * init hook
 * @return success or not
 */
bool tls_hook_init();

/**
 * hook function
 * @param symbol: function name
 * @param new_func: function you want to replace with
 * @param old_func: origin function entry point
 * @return success or not
 */
bool tls_hook_func(const char *symbol, void *new_func, void **old_func);

/**
 * clear all hooks
 */
void tls_hook_clear();

}
