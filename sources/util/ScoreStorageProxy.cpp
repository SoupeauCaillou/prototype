#include "ScoreStorageProxy.h"

#include "util/StorageProxy.h"

#include <base/Log.h>

#include <algorithm>

ScoreStorageProxy::ScoreStorageProxy() {
    _tableName = "Score";
    _columnsNameAndType["points"] = "int";
    _columnsNameAndType["coins"] = "int";
    _columnsNameAndType["name"] = "string";
}

std::string ScoreStorageProxy::getValue(const std::string& columnName) {
    if (columnName == "points") {
        return StorageProxy::int2sql(_queue.back().points);
    } else if (columnName == "coins") {
        return StorageProxy::int2sql(_queue.back().coins);
    } else if (columnName == "name") {
        return _queue.back().name;
    } else {
        LOGW("No such table name: " << columnName);
    }
    return "";
}

void ScoreStorageProxy::setValue(const std::string& columnName, const std::string& value) {
    if (columnName == "points") {
        _queue.back().points = StorageProxy::sql2int(value);
    } else if (columnName == "coins") {
         _queue.back().coins = StorageProxy::sql2int(value);
    } else if (columnName == "name") {
        _queue.back().name = value;
    } else {
        LOGW("No such table name: " << columnName);
    }
}
