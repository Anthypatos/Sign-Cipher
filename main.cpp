#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <utility>
#include <fstream>
#include <ios>
#include <cstddef>
#include <cstdio>
#include <stdexcept>


std::string Encrypt(const std::string& CsMessage, int iKey,
    const std::unordered_map<char, int>& ChtAlphaToIndex, const std::unordered_map<int, char>& ChtIndexToAlpha);
std::string Decrypt(const std::string& CsMessage, int iKey,
    const std::unordered_map<char, int>& ChtAlphaToIndex, const std::unordered_map<int, char>& ChtIndexToAlpha);
std::string Sign(const std::string& CsMessage);


int main(int iArgNumber, char** asArgs)
{
    if (iArgNumber < 2)
    {
        std::cout << "Unrecognized command." << std::endl <<
            "Type 'signcipher -h' for help" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string CsAlphabet{"abcdefghijklmnopqrstuvwxyz"};
    std::unordered_map<char, int> htAlphaToIndex{};
    std::unordered_map<int, char> htIndexToAlpha{};

    for (int i = 0; CsAlphabet[i]; i++)
    {
        htAlphaToIndex.insert(std::make_pair(CsAlphabet[i], i));
        htIndexToAlpha.insert(std::make_pair(i, CsAlphabet[i]));
    }

    if (iArgNumber == 2)
    {
        if (!(std::strcmp(asArgs[1], "-h")))
        {
            std::cout << "Usage: signcipher [OPTION] {FILE | HASH}" << std::endl <<
                "-e string\tEncrypt string" << std::endl <<
                "-d string\tDecrypt string" << std::endl <<
                "-s string\tSign string" << std::endl <<
                "-f file hash\tCompares a hash against the passwords in file" << std::endl <<
                "-h\tShow this help" << std::endl;
        }
    }
    else if (iArgNumber == 3)
    {
        if (!(std::strcmp(asArgs[1], "-e")))
        {
            std::cout << Encrypt(asArgs[2], std::strlen(asArgs[2]), htAlphaToIndex, htIndexToAlpha) << std::endl;
        }
        else if (!(std::strcmp(asArgs[1], "-d")))
        {
            std::cout << Decrypt(asArgs[2], std::strlen(asArgs[2]), htAlphaToIndex, htIndexToAlpha) << std::endl;
        }
        else if (!(std::strcmp(asArgs[1], "-s")))
        {
            std::cout << Sign(asArgs[2]) << std::endl;
        }
    }
    else if (iArgNumber == 4)
    {
        if (!(std::strcmp(asArgs[1], "-f")))
        {
            std::filesystem::path pathFile(asArgs[2]);
            if (!std::filesystem::exists(pathFile) || !std::filesystem::is_regular_file(pathFile))
            {
                std::cout << "Error opening file." << std::endl;
                return EXIT_FAILURE;
            }

            std::ifstream ifstreamFile(asArgs[2], std::ios_base::in);
            std::string sPassword{};

            bool bIsFound = false;
            while (ifstreamFile >> sPassword && !bIsFound)
            {
                std::string sHash = Sign(Encrypt(sPassword, sPassword.length(), htAlphaToIndex, htIndexToAlpha));
                std::cout << sPassword << " " << sHash;
                if (sHash == asArgs[3])
                {
                    std::cout << " OK ";
                    bIsFound = true;
                }
                std::cout << std::endl;
            }
        }
    }
    else
    {
        std::cout << "Unrecognized command." << std::endl <<
            "Type 'signcipher -h' for help" << std::endl;
        return EXIT_FAILURE;
    }
}


std::string Encrypt(const std::string& CsMessage, int iKey,
    const std::unordered_map<char, int>& ChtAlphaToIndex, const std::unordered_map<int, char>& ChtIndexToAlpha)
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
    const std::unordered_map<char, int>& ChtAlphaToIndex, const std::unordered_map<int, char>& ChtIndexToAlpha)
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
