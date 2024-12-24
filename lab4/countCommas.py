inputStr = input("Enter a string: ")
count = 0
for i in inputStr:
    if i == ',':
        count += 1
print(count)