# Automated test script to verify full workflow
$inputLines = @(
    # 1. Check seat map of 10303 (3 seats total)
    "6",
    "10303",
    # 2. Book seat 1
    "4",
    "10303",
    "Amit Sharma",
    "28",
    "M",
    "12", "11", "2026",
    # 3. Book seat 2
    "4",
    "10303",
    "Priya Patel",
    "24",
    "F",
    "12", "11", "2026",
    # 4. Book seat 3 (now full)
    "4",
    "10303",
    "Rohan Verma",
    "35",
    "M",
    "12", "11", "2026",
    # 5. Book 4th ticket (should go to waiting list queue!)
    "4",
    "10303",
    "Sneha Rao",
    "22",
    "F",
    "12", "11", "2026",
    # 6. View seat map of 10303 (all [XX])
    "6",
    "10303",
    # 7. View waiting list (Sneha should be at WL-1)
    "8",
    # 8. View all passengers of 10303
    "7",
    "2",
    "10303",
    # 9. Cancel ticket for Amit (PNR 1001) -> Should auto-promote Sneha!
    "5",
    "1001",
    "Y",
    # 10. View passenger list again to verify Sneha is now CONFIRMED
    "7",
    "2",
    "10303",
    # 11. View waiting list (should now be empty!)
    "8",
    # 0. Exit
    "0"
)

$inputString = ($inputLines -join "`r`n") + "`r`n"
$output = $inputString | .\railway.exe
$output | Out-File -FilePath "test_output.txt" -Encoding utf8
Write-Host "Test completed! Output written to test_output.txt"
