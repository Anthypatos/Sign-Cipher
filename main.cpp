#include <string>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <utility>
#include <fstream>
#include <ios>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <stdexcept>


std::string Encrypt(const std::string& CsMessage, int iKey,
    const std::map<char, int>& ChtAlphaToIndex, const std::map<int, char>& ChtIndexToAlpha);
std::string Decrypt(const std::string& CsMessage, int iKey,
    const std::map<char, int>& ChtAlphaToIndex, const std::map<int, char>& ChtIndexToAlpha);
std::string Sign(const std::string& CsMessage);


int main(int iArgNumber, char** asArgs)
{
    if (iArgNumber != 2)
    {
        std::cout << "Unrecognized command." << std::endl <<
            "Usage: signcipher [FILE]";
        return EXIT_FAILURE;
    }

    std::filesystem::path pathFile(asArgs[1]);
    if (!std::filesystem::exists(pathFile) || !std::filesystem::is_regular_file(pathFile))
    {
        std::cout << "File does not exist." << std::endl;
        return EXIT_FAILURE;
    }

    const std::string CsAlphabet{"abcdefghijklmnopqrstuvwxyz"};
    std::map<char, int> htAlphaToIndex{};
    std::map<int, char> htIndexToAlpha{};

    for (int i = 0; CsAlphabet[i]; i++)
    {
        htAlphaToIndex.insert(std::make_pair(CsAlphabet[i], i));
        htIndexToAlpha.insert(std::make_pair(i, CsAlphabet[i]));
    }

    std::ifstream ifstreamFile(asArgs[1], std::ios_base::in);
    std::string sPassword{};

    bool bIsFound = false;
    while (ifstreamFile >> sPassword && !bIsFound)
    {
        std::string sTemp = Sign(Encrypt(sPassword, sPassword.length(), htAlphaToIndex, htIndexToAlpha));
        std::cout << sPassword << " " << sTemp;
        if (sTemp == "cc7d9b70e41784614a6bfac7c047262c")
        {
            std::cout << " OK " << sPassword;
            bIsFound = true;
        }
        std::cout << std::endl;
    }

    //for (int i = 0; asArgs[1][i]; i++) asArgs[1][i] = std::tolower(asArgs[1][i]);

    //std::cout << Encrypt(asArgs[1], 4, htAlphaToIndex, htIndexToAlpha) << std::endl;
    //std::cout << Decrypt(asArgs[1], 4, htAlphaToIndex, htIndexToAlpha) << std::endl;
    //std::cout << Sign(asArgs[1]) << std::endl;
}


std::string Encrypt(const std::string& CsMessage, int iKey,
    const std::map<char, int>& ChtAlphaToIndex, const std::map<int, char>& ChtIndexToAlpha)
{
    std::string sCipherText{};

    for (size_t i = 0; i < CsMessage.length(); i++)
    {
        int iCipherIndex = (ChtAlphaToIndex.at(CsMessage[i]) + iKey) % ChtAlphaToIndex.size();
        sCipherText.push_back(ChtIndexToAlpha.at(iCipherIndex));
        iKey = iCipherIndex;
    }

    return sCipherText;
}


std::string Decrypt(const std::string& CsMessage, int iKey,
    const std::map<char, int>& ChtAlphaToIndex, const std::map<int, char>& ChtIndexToAlpha)
{
    std::string sPlainText{};
    int iCurrentPos = ChtAlphaToIndex.at(CsMessage.back());

    for (size_t i = CsMessage.length() - 1; i > 0; i--)
    {
        int iCipherIndex = ChtAlphaToIndex.at(CsMessage[i - 1]);
        sPlainText.insert(sPlainText.cbegin(),
            ChtIndexToAlpha.at((iCurrentPos + ChtIndexToAlpha.size() - iCipherIndex) % ChtIndexToAlpha.size()));
        iCurrentPos = iCipherIndex;
    }
    sPlainText.insert(sPlainText.cbegin(),
        ChtIndexToAlpha.at((iCurrentPos + ChtIndexToAlpha.size() - iKey) % ChtIndexToAlpha.size()));

    return sPlainText;
}


std::string Sign(const std::string& CsMessage)
{
    FILE* pFilePipe;

    if ((pFilePipe = static_cast<FILE*>(
        popen(("echo -n \"" + CsMessage + "\" | md5sum").c_str(), "r"))) == nullptr)
        throw std::runtime_error("popen() failed");

    char sHash[33] = "";
    if ((std::fgets(sHash, 32, pFilePipe)) == nullptr) throw std::runtime_error("Command failed");

    if (pclose(pFilePipe)) throw std::runtime_error("Command failed");

    return std::string(sHash);
}
