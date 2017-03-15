HAL creation
============

If your MCU is not already compatible with this librairy you have to create your hown HAL (Hardware Abstraction Layer).

The creation of a new HAL is a big contribution to this project and you will have to pull-request it in the future. To do a good pull request create a new git branch at the poppy_com master level and put all your code inside.
When your new HAL is fully operational please pullrequest your branch to contribute to the poppy-project.

Start your new hal by creating a new folder on poppy-com/hal/

You can choose the name of your MCU on your Makefile but you have to give exactly the same name to your new poppy-com/hal/MCU folder.

You can use the HAL template in poppy-com/hal/template/

If you have any question about hal creation please ask!
