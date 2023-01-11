#pragma once
#include <string>
/*!
    \defgroup lamoda Парсер сайта lamoda.ru
    \brief Данная библиотека предназначена для парсинга сайта lamoda.ru на предмет наличия скидки на конкретный товар
*/
#ifdef _WIN32
extern "C" __declspec(dllexport) bool parse(std::string & filecontents);
#else
///@{
/// <summary>
/// Метод, парсящий страницу с товаром и определяющий, есть ли на него скидка
/// </summary>
/// <param name="filecontents">HTML верстка страницы</param>
/// <returns>Возращает true в случае обнаружения скидки, иначе false</returns>
extern "C" bool parse(std::string & filecontents);
///@}
#endif