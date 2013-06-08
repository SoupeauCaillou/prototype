#include "ScoreStorageProxy.h"

#include "base/Log.h"
#include "base/ObjectSerializer.h"

ScoreStorageProxy::ScoreStorageProxy() {
    _tableName = "Score";
    _columnsNameAndType["time"] = "float";
    _columnsNameAndType["name"] = "string";
}

std::string ScoreStorageProxy::getValue(const std::string& columnName) {
    if (columnName == "time") {
        return ObjectSerializer<float>::object2string(_queue.back().time);
    } else if (columnName == "name") {
        return _queue.back().name;
    } else {
        LOGW("No such column name: " << columnName);
    }
    return "";
}

void ScoreStorageProxy::setValue(const std::string& columnName, const std::string& value, bool pushNewElement) {
    if (pushNewElement) {
        pushAnElement();
    }

    if (columnName == "time") {
        _queue.back().time =  ObjectSerializer<float>::string2object(value);
    } else if (columnName == "name") {
        _queue.back().name = value;
    } else {
        LOGW("No such column name: " << columnName);
    }
}