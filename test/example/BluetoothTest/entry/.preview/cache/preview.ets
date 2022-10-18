struct preview{build(){Row() {
                Checkbox({ name: 'txPower', group: 'checkboxGroup' })
                  .select(this.cbxTxPower)
                  .selectedColor(0x39a2db)
                  .onChange((value) => {
                    if(value)
                    {
                      this.cbxTxPower =true;
                    }
                    else
                    {
                      this.cbxTxPower =false;
                    }
                  }).width("15vp").margin( { left: "1vp" } )
                Text("txPower").fontSize(14).width(80)
                TextInput({ text: this.advSettings_txPower.toString(), placeholder: "input txPower" })
                  .fontSize(14)
                  .onChange((strInput) => {
                    this.advSettings_txPower = strInput;
                  })
                  .width("60%")
                  .borderRadius(1)
              }
              .height(40)
              .padding({top: 5})
              .justifyContent(FlexAlign.Start)
              .alignItems(VerticalAlign.Center)}}