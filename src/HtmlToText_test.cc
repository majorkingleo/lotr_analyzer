#include <iostream>
#include <cassert>
#include "HtmlToText.h"

int main()
{
    int passed = 0;
    int failed = 0;
    
    // Test 1: Named entities (nbsp)
    {
        std::wstring input = L"<p>Hello &nbsp; World</p>";
        std::wstring expected = L"Hello World";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::wcout << L"✓ Test 1 PASSED" << std::endl;
            passed++;
        } else {
            std::wcout << L"✗ Test 1 FAILED" << std::endl;
            std::wcout << L"  Expected: [" << expected << L"]" << std::endl;
            std::wcout << L"  Got:      [" << result << L"]" << std::endl;
            failed++;
        }
    }
    
    // Test 2: Comparison entities (lt, gt)
    {
        std::wstring input = L"<div>Foo &lt; Bar &gt; Baz</div>";
        std::wstring expected = L"Foo < Bar > Baz";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::wcout << L"✓ Test 2 PASSED" << std::endl;
            passed++;
        } else {
            std::wcout << L"✗ Test 2 FAILED" << std::endl;
            std::wcout << L"  Expected: [" << expected << L"]" << std::endl;
            std::wcout << L"  Got:      [" << result << L"]" << std::endl;
            failed++;
        }
    }
    
    // Test 3: Numeric entities
    {
        std::wstring input = L"&#847;&zwnj;&nbsp;&#847;&zwnj;&nbsp;Test";
        std::wstring expected = L"\u034F\u200C \u034F\u200C Test";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::wcout << L"✓ Test 3 PASSED" << std::endl;
            passed++;
        } else {
            std::wcout << L"✗ Test 3 FAILED" << std::endl;
            std::wcout << L"  Expected: [" << expected << L"]" << std::endl;
            std::wcout << L"  Got:      [" << result << L"]" << std::endl;
            failed++;
        }
    }
    
    // Test 4: Script tag removal and ampersand
    {
        std::wstring input = L"<script>alert('test');</script><p>Real Content &amp; More</p>";
        std::wstring expected = L"Real Content & More";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::wcout << L"✓ Test 4 PASSED" << std::endl;
            passed++;
        } else {
            std::wcout << L"✗ Test 4 FAILED" << std::endl;
            std::wcout << L"  Expected: [" << expected << L"]" << std::endl;
            std::wcout << L"  Got:      [" << result << L"]" << std::endl;
            failed++;
        }
    }
    
    // Test 5: Hexadecimal entities
    {
        std::wstring input = L"&#x00A9; 2026 &#x00AE;";
        std::wstring expected = L"© 2026 ®";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::wcout << L"✓ Test 5 PASSED" << std::endl;
            passed++;
        } else {
            std::wcout << L"✗ Test 5 FAILED" << std::endl;
            std::wcout << L"  Expected: [" << expected << L"]" << std::endl;
            std::wcout << L"  Got:      [" << result << L"]" << std::endl;
            failed++;
        }
    }
    
    // Test 6: Comments and multiple tags
    {
        std::wstring input = L"<!-- Comment --><p>Text</p><br/><span>More</span>";
        std::wstring expected = L"Text More";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::wcout << L"✓ Test 6 PASSED" << std::endl;
            passed++;
        } else {
            std::wcout << L"✗ Test 6 FAILED" << std::endl;
            std::wcout << L"  Expected: [" << expected << L"]" << std::endl;
            std::wcout << L"  Got:      [" << result << L"]" << std::endl;
            failed++;
        }
    }
    
    std::wcout << std::endl;
    std::wcout << L"Results: " << passed << L" passed, " << failed << L" failed" << std::endl;
    
    return (failed == 0) ? 0 : 1;
}
