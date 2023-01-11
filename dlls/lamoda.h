#pragma once
#include <string>
/*!
    \defgroup lamoda ѕарсер сайта lamoda.ru
    \brief ƒанна€ библиотека предназначена дл€ парсинга сайта lamoda.ru на предмет наличи€ скидки на конкретный товар
*/
#ifdef _WIN32
extern "C" __declspec(dllexport) bool parse(std::string & filecontents);
#else
///@{
/// <summary>
/// ћетод, парс€щий страницу с товаром и определ€ющий, есть ли на него скидка
/// </summary>
/// <param name="filecontents">HTML верстка страницы</param>
/// <returns>¬озращает true в случае обнаружени€ скидки, иначе false</returns>
extern "C" bool parse(std::string & filecontents);
///@}
#endif