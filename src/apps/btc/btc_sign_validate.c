// Copyright 2020 Shift Crypto AG
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "btc_sign_validate.h"
#include "btc_common.h"
#include "btc_params.h"
#include "confirm_multisig.h"

#include <memory/memory.h>

app_btc_result_t app_btc_sign_validate_init_script_configs(
    BTCCoin coin,
    const BTCScriptConfig* script_config,
    const uint32_t* keypath_account,
    size_t keypath_account_count)
{
    const app_btc_coin_params_t* coin_params = app_btc_params_get(coin);
    if (coin_params == NULL) {
        return APP_BTC_ERR_INVALID_INPUT;
    }
    switch (script_config->which_config) {
    case BTCScriptConfig_simple_type_tag:
        break;
    case BTCScriptConfig_multisig_tag: {
        const BTCScriptConfig_Multisig* multisig = &script_config->config.multisig;
        if (!btc_common_multisig_is_valid(
                multisig, keypath_account, keypath_account_count, coin_params->bip44_coin)) {
            return APP_BTC_ERR_INVALID_INPUT;
        }
        uint8_t multisig_hash[SHA256_LEN] = {0};
        if (!btc_common_multisig_hash(
                coin, multisig, keypath_account, keypath_account_count, multisig_hash)) {
            return APP_BTC_ERR_INVALID_INPUT;
        };
        char multisig_registered_name[MEMORY_MULTISIG_NAME_MAX_LEN] = {0};
        if (!memory_multisig_get_by_hash(multisig_hash, multisig_registered_name)) {
            // Not previously registered -> fail.
            return APP_BTC_ERR_INVALID_INPUT;
        }
        if (!apps_btc_confirm_multisig(
                "Spend from", coin, multisig_registered_name, multisig, false)) {
            return APP_BTC_ERR_USER_ABORT;
        }
        break;
    }
    default:
        return APP_BTC_ERR_INVALID_INPUT;
    }
    return APP_BTC_OK;
}