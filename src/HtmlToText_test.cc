#include <iostream>
#include <cassert>
#include "HtmlToText.h"
#include <utf8_util.h>
#include <read_file.h>
#include <OutDebug.h>

using namespace Tools;

int run_tests()
{
    int passed = 0;
    int failed = 0;
    
    // Test 1: Named entities (nbsp)
    {
        std::wstring input = L"<p>Hello &nbsp; World</p>";
        std::wstring expected = L"Hello World";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::cout << Utf8Util::wStringToUtf8(L"✓ Test 1 PASSED") << std::endl;
            passed++;
        } else {
            std::cout << Utf8Util::wStringToUtf8(L"✗ Test 1 FAILED") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Expected: [") << Utf8Util::wStringToUtf8(expected) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Got:      [") << Utf8Util::wStringToUtf8(result) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            failed++;
        }
    }
    
    // Test 2: Comparison entities (lt, gt)
    {
        std::wstring input = L"<div>Foo &lt; Bar &gt; Baz</div>";
        std::wstring expected = L"Foo < Bar > Baz";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::cout << Utf8Util::wStringToUtf8(L"✓ Test 2 PASSED") << std::endl;
            passed++;
        } else {
            std::cout << Utf8Util::wStringToUtf8(L"✗ Test 2 FAILED") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Expected: [") << Utf8Util::wStringToUtf8(expected) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Got:      [") << Utf8Util::wStringToUtf8(result) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            failed++;
        }
    }
    
    // Test 3: Numeric entities
    {
        std::wstring input = L"&#847;&zwnj;&nbsp;&#847;&zwnj;&nbsp;Test";
        std::wstring expected = L"\u034F\u200C \u034F\u200C Test";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::cout << Utf8Util::wStringToUtf8(L"✓ Test 3 PASSED") << std::endl;
            passed++;
        } else {
            std::cout << Utf8Util::wStringToUtf8(L"✗ Test 3 FAILED") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Expected: [") << Utf8Util::wStringToUtf8(expected) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Got:      [") << Utf8Util::wStringToUtf8(result) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            failed++;
        }
    }
    
    // Test 4: Script tag removal and ampersand
    {
        std::wstring input = L"<script>alert('test');</script><p>Real Content &amp; More</p>";
        std::wstring expected = L"Real Content & More";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::cout << Utf8Util::wStringToUtf8(L"✓ Test 4 PASSED") << std::endl;
            passed++;
        } else {
            std::cout << Utf8Util::wStringToUtf8(L"✗ Test 4 FAILED") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Expected: [") << Utf8Util::wStringToUtf8(expected) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Got:      [") << Utf8Util::wStringToUtf8(result) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            failed++;
        }
    }
    
    // Test 5: Hexadecimal entities
    {
        std::wstring input = L"&#x00A9; 2026 &#x00AE;";
        std::wstring expected = L"© 2026 ®";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::cout << Utf8Util::wStringToUtf8(L"✓ Test 5 PASSED") << std::endl;
            passed++;
        } else {
            std::cout << Utf8Util::wStringToUtf8(L"✗ Test 5 FAILED") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Expected: [") << Utf8Util::wStringToUtf8(expected) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Got:      [") << Utf8Util::wStringToUtf8(result) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            failed++;
        }
    }
    
    // Test 6: Comments and multiple tags
    {
        std::wstring input = L"<!-- Comment --><p>Text</p><br/><span>More</span>";
        std::wstring expected = L"Text More";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::cout << Utf8Util::wStringToUtf8(L"✓ Test 6 PASSED") << std::endl;
            passed++;
        } else {
            std::cout << Utf8Util::wStringToUtf8(L"✗ Test 6 FAILED") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Expected: [") << Utf8Util::wStringToUtf8(expected) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Got:      [") << Utf8Util::wStringToUtf8(result) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            failed++;
        }
    }
    
    // Test 7: Quoted-printable encoding (e.g., F=C3=BCr -> Für)
    {
        std::wstring input = L"F=C3=BCr die Kinos";
        std::wstring expected = L"Für die Kinos";
        std::wstring result = HtmlToText::convert(input);
        if (result == expected) {
            std::cout << Utf8Util::wStringToUtf8(L"✓ Test 7 PASSED") << std::endl;
            passed++;
        } else {
            std::cout << Utf8Util::wStringToUtf8(L"✗ Test 7 FAILED") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Expected: [") << Utf8Util::wStringToUtf8(expected) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            std::cout << Utf8Util::wStringToUtf8(L"  Got:      [") << Utf8Util::wStringToUtf8(result) << Utf8Util::wStringToUtf8(L"]") << std::endl;
            failed++;
        }
    }
    
    std::cout << std::endl;
    std::cout << Utf8Util::wStringToUtf8(L"Results: ") << passed << Utf8Util::wStringToUtf8(L" passed, ") << failed << Utf8Util::wStringToUtf8(L" failed") << std::endl;
    
    return (failed == 0) ? 0 : 1;
}


int main( int argc, char* argv[] )
{
    Tools::x_debug = new OutDebug();

    if( argc == 1 ) {
        return run_tests();
    }

    if( argc > 1 ) {

        std::wstring input;

        if( !READ_FILE.read_file( argv[1], input ) ) {
            std::cerr << "Failed to read file: " << READ_FILE.getError() << std::endl;
            return 1;
        }



        std::wstring output = HtmlToText::convert_from_mail(input);
        std::cout << Utf8Util::wStringToUtf8(output) << std::endl;
    }
}