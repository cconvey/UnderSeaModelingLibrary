/**
 * @file managed.h Managed Objects
 * @defgroup managed managed
 *
 * Thread-safe map to store and manage dynamic objects. Searches for these
 * entries using the keyID field of the object to be found. Duplicate keys are
 * not allowed. Event listeners are notified when objects are added to the
 * manager, removed from the manager, or updated.
 *
 * @defgroup managed_test Managed Tests
 * @ingroup managed
 *
 * Regression tests for the managed package
 */
#pragma once

#include <managed/managed_obj.h>
#include <managed/manager_listener.h>
#include <managed/manager_template.h>
#include <managed/update_listener.h>
#include <managed/update_notifier.h>

