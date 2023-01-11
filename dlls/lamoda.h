#pragma once
#include <string>
/*!
    \defgroup lamoda ������ ����� lamoda.ru
    \brief ������ ���������� ������������� ��� �������� ����� lamoda.ru �� ������� ������� ������ �� ���������� �����
*/
#ifdef _WIN32
extern "C" __declspec(dllexport) bool parse(std::string & filecontents);
#else
///@{
/// <summary>
/// �����, �������� �������� � ������� � ������������, ���� �� �� ���� ������
/// </summary>
/// <param name="filecontents">HTML ������� ��������</param>
/// <returns>��������� true � ������ ����������� ������, ����� false</returns>
extern "C" bool parse(std::string & filecontents);
///@}
#endif