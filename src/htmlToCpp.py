with open('web.html', 'r') as file:
    webFileData = file.read()
with open('internetManagement.cpp', 'r') as file:
    initualFileData = file.read()

index = initualFileData.find("<!--BEGIN-->", 0) 
if index != -1:
    index += 12
    initualFileDataFirstIndex = index
    counter = 0
    while index != -1:
        index = initualFileData.find("<!--", index)+4 
        lastIndex = initualFileData.find("-->", index) 
        topic = initualFileData[index:lastIndex]
        
        if topic == "END":
            break
        
        beginIndexInitualFile = lastIndex+4
        lastIndexInitualFile = initualFileData.find("<!--"+topic, lastIndex)+7+len(topic)
        
        if (lastIndexInitualFile != -1):
            beginIndexWebFile = webFileData.find(topic, 0)+4
            lastIndexWebFile = webFileData.find(topic, beginIndexWebFile)
            with open('internetManagement.cpp', 'w') as file:
                if counter == 0:
                    
                else:
                    
            counter += 1
            print(beginIndexWebFile)
            print(topic)
            print(lastIndexWebFile)
            
        index += len(topic)
else:
    if initualFileData.find("<!--", 0) == -1:
        print("Can`t find any comments that begin with \"<!--\" in cpp file")
    else:
        print("Can`t find any \"<!--BEGIN-->\" in cpp file")


