--[[
	Part 1:
	Called from c++
]]

function onLoad()
	print("Hello from onLoad")

	--MakeLuaStateWithComplexObjects()
	local w = widgets.Window({size={x=300,y=300}})




		w:AddComponent(widgets.Label({
			str = "Amostra ip:",
			id = "testlabel",
			alignment = { top = 'parent.top',left = 'parent.left', },
		}))

		w:AddComponent(widgets.TextInput({
			alignment = { top = 'parent.top + 32',left = 'parent.left + 4' },
			str = "192.168.25.12",
			id = "ipfield",
		}))

		w:AddComponent(widgets.TextInput({
			alignment = { left = 'ipfield.left',top = 'ipfield.bottom + 8' },
			str = "name",
			id = "inputname",
		}))

		w:AddComponent(widgets.Button({
			alignment = { left = 'ipfield.right + 16',top = 'connect.bottom + 4' },
			str = "Host",
			action = function(self,button)
				g_state.Signal({int=777})
				g_state.Signal({str=w:GetChildById("inputname"):GetText()})
				if g_state.Signal({int=1}) then
					w:Destroy()
				end
			end
		}))

		w:AddComponent(widgets.Button({
			alignment = { left = 'ipfield.right + 16',top = 'ipfield.top' },
			str = "Connect",
			id = "connect",
			action = function(self,button)
				g_state.Signal({int=777})
				g_state.Signal({str=w:GetChildById("inputname"):GetText()})
				local ret = g_state.Signal({str= w:GetChildById("ipfield"):GetText()})
				if ret then
					w:Destroy()
				end
			end
		}))


		w:AddComponent(widgets.Button({
			alignment = { bottom = 'parent.bottom',left = 'parent.left' },
			str = "Close",
			action = function() g_state.Signal({int=2}) end
		}))

		g_ui.AddWindow(w)

end



function MakeLuaStateWithComplexObjects()
	local state = LuaGameState()


	--Some UI




	state.timer = 200.0

	state.text = Text("owo",12,{r=255,g=255,b=0,a=100})



	state.OnBegin = function(this)
		--Should initiate the camera system. Everything outside screen is ignored
		g_camera.Initiate()

		local w = LuaUi()

		--[[w:AddComponent(widgets.Label({
			str = "Amostra ip:",
			id = "testlabel",
			alignment = { top = 'parent.top',left = 'parent.left', },
		}))]]
		g_ui.AddWindow(w)





		--[[w:AddComponent(widgets.Label({
			str = "Label",
			id = "testlabel",
			alignment = { top = 'parent.top',left = 'parent.left', },
		}))

		w:AddComponent(widgets.Button({
			str = "Button",
			action = function (thisButton,mouseKey)
				print("Pressed!!!!!!!")
				MakeObject()
				w:GetChildById('testlabel'):SetText("Pressed")
			end,
			alignment = { top = 'testlabel.bottom + 5',left = 'parent.left + 2', },
		}))

		w:AddComponent(widgets.Button({
			str = "Next test",
			action = function (thisButton,mouseKey)
				this.data.canClose = true
			end,
			alignment = { bottom = 'parent.bottom -4',right = 'parent.right -4', },
		}))

		g_ui.AddWindow(w)
		this.w = w;

		MakeObject()]]
	end

	state.OnFinish = function(this)
		--g_assets.EraseResource("test")
	end

	state.OnResume = function(this)

	end

	state.OnPause = function(this)

	end

	state.OnUpdate = function(this,dt)
		this.timer = this.timer - dt
		if this.timer <= 0 then
			this.data.canClose = true
		end
		this.text:SetText(tostring(this.timer))
	end

	state.OnRender = function(this)
		g_render.DrawFillSquare({x=0,y=0,w=640,h=480},0,0,state.timer,255)

		this.text:Render({x=32,y=200})

	end



	state:Setup()
end
