#define CATCH_CONFIG_MAIN
#define TESTING  // Define TESTING before including QEditor.h
#include <catch2/catch_test_macros.hpp>
#include "../src/QEditor.h"

// Helper function to create a test-ready editor
Editor createTestEditor() {
    Editor editor;
    editor.skipTerminalInit();
    editor.setMockTerminalSize(24, 80);
    return editor;
}

TEST_CASE("Basic editor initialization", "[editor]") {
    Editor editor = createTestEditor();
    REQUIRE(editor.isInNormalMode());
    REQUIRE(editor.getBuffer().empty());
    REQUIRE(editor.getCursorX() == 0);
    REQUIRE(editor.getCursorY() == 0);
}

TEST_CASE("Editor mode switching", "[editor]") {
    Editor editor = createTestEditor();
    
    SECTION("Start in normal mode") {
        REQUIRE(editor.isInNormalMode());
        REQUIRE_FALSE(editor.isInEditMode());
    }
    
    SECTION("Switch to edit mode") {
        editor.editMode();
        REQUIRE(editor.isInEditMode());
        REQUIRE_FALSE(editor.isInNormalMode());
    }
    
    SECTION("Switch back to normal mode") {
        editor.editMode();
        editor.normalMode();
        REQUIRE(editor.isInNormalMode());
        REQUIRE_FALSE(editor.isInEditMode());
    }
}

TEST_CASE("Editor buffer operations", "[editor]") {
    Editor editor = createTestEditor();
    
    SECTION("Empty buffer") {
        REQUIRE(editor.getBuffer().empty());
    }
}

TEST_CASE("Editor cursor navigation", "[editor]") {
    Editor editor = createTestEditor();
    
    // Set up a 3x3 grid of text for testing navigation
    editor.editMode();
    editor.insertText('1'); editor.insertText('2'); editor.insertText('3');
    editor.insertNewline();
    editor.insertText('4'); editor.insertText('5'); editor.insertText('6');
    editor.insertNewline();
    editor.insertText('7'); editor.insertText('8'); editor.insertText('9');
    editor.normalMode();
    
    SECTION("Move right (l)") {
        editor.setCursorPosition(0, 0);  // Start at top-left
        editor.moveCursor('l');
        REQUIRE(editor.getCursorX() == 1);
        REQUIRE(editor.getCursorY() == 0);
    }
    
    SECTION("Move left (h)") {
        editor.setCursorPosition(1, 0);  // Start at position (1,0)
        editor.moveCursor('h');
        REQUIRE(editor.getCursorX() == 0);
        REQUIRE(editor.getCursorY() == 0);
    }
    
    SECTION("Move down (j)") {
        editor.setCursorPosition(1, 0);  // Start at position (1,0)
        editor.moveCursor('j');
        REQUIRE(editor.getCursorX() == 1);
        REQUIRE(editor.getCursorY() == 1);
    }
    
    SECTION("Move up (k)") {
        editor.setCursorPosition(1, 1);  // Start at position (1,1)
        editor.moveCursor('k');
        REQUIRE(editor.getCursorX() == 1);
        REQUIRE(editor.getCursorY() == 0);
    }
    
    SECTION("Boundary conditions") {
        // Test moving left at left edge
        editor.setCursorPosition(0, 0);
        editor.moveCursor('h');
        REQUIRE(editor.getCursorX() == 0);  // Should not move
        
        // Test moving right at right edge
        editor.setCursorPosition(2, 0);
        editor.moveCursor('l');
        REQUIRE(editor.getCursorX() == 2);  // Should not move
        
        // Test moving up at top edge
        editor.setCursorPosition(1, 0);
        editor.moveCursor('k');
        REQUIRE(editor.getCursorY() == 0);  // Should not move
        
        // Test moving down at bottom edge
        editor.setCursorPosition(1, 2);
        editor.moveCursor('j');
        REQUIRE(editor.getCursorY() == 2);  // Should not move
    }
    
    SECTION("Navigation across different line lengths") {
        // Add a shorter line
        editor.editMode();
        editor.setCursorPosition(0, 3);
        editor.insertText('a');
        editor.insertText('b');
        editor.normalMode();
        
        // Test moving down to shorter line
        editor.setCursorPosition(2, 2);  // At position (2,2)
        editor.moveCursor('j');
        REQUIRE(editor.getCursorX() == 1);  // Should adjust to shorter line
        REQUIRE(editor.getCursorY() == 3);
        
        // Test moving up to longer line
        editor.setCursorPosition(1, 3);  // At position (1,3)
        editor.moveCursor('k');
        REQUIRE(editor.getCursorX() == 1);  // Should maintain x position
        REQUIRE(editor.getCursorY() == 2);
    }
} 