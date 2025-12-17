#pragma once
#include <string>
#include <variant>
#include <map>

#include <fstream>    // ファイルI/Oのために必要
#include <iostream>   // エラーメッセージなどの出力のために必要
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>

class JsonUtils
{
public:
    // 書き込み
    template<class Temp>
    static void SaveJsonFile(const Temp& data, const std::string& filename)
    {
        // 出力ファイルストリームを作成
        std::ofstream os(filename);

        // ファイルが正常に開かれたかチェック
        if (!os.is_open())
        {
            return;
        }
        // JSONOutputArchive を作成し、ファイルストリームに関連付ける
        cereal::JSONOutputArchive archive(os);

        // NodeJsonData オブジェクトをシリアライズ
        archive(CEREAL_NVP(data));
    }
    //書き込み(shared_ptr)
    template<class Temp>
    static void SaveJsonFile(const std::shared_ptr<Temp> data, const std::string& filename)
    {
        // 出力ファイルストリームを作成
        std::ofstream os(filename);

        // ファイルが正常に開かれたかチェック
        if (!os.is_open())
        {
            return;
        }
        // JSONOutputArchive を作成し、ファイルストリームに関連付ける
        cereal::JSONOutputArchive archive(os);

        // NodeJsonData オブジェクトをシリアライズ
        archive(CEREAL_NVP(data));
    }

    // 読み込み: データを変更するので非const参照にする。成功/失敗を示す bool を返すように変更
    template<class Temp>
    static bool LoadJsonFile(Temp& data, const std::string& filename)
    {
        // 入力ファイルストリームを作成
        std::ifstream is(filename);

        // ファイルが正常に開かれたかチェック
        if (!is.is_open())
        {
            return false; // 失敗
        }

        try
        {
            // JSONInputArchive を作成し、ファイルストリームに関連付ける
            cereal::JSONInputArchive archive(is);
            // NodeJsonData オブジェクトをデシリアライズ
            archive(CEREAL_NVP(data));
            return true; // 成功
        }
        catch (const cereal::Exception& e)
        {
            return false; // 失敗
        }
    }

    //読み込み(shared_ptr)
    template<class Temp>
    static bool LoadJsonFile(std::shared_ptr<Temp>& data, const std::string& filename)
    {
        // 入力ファイルストリームを作成
        std::ifstream is(filename);

        // ファイルが正常に開かれたかチェック
        if (!is.is_open())
        {
            return false; // 失敗
        }

        try
        {
            // JSONInputArchive を作成し、ファイルストリームに関連付ける
            cereal::JSONInputArchive archive(is);

            // NodeJsonData オブジェクトをデシリアライズ
            archive(CEREAL_NVP(data));

            return true; // 成功
        }
        catch (const cereal::Exception& e)
        {
            return false; // 失敗
        }
    }
};