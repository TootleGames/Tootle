#include "TSceneNode_Emitter.h"
#include <TootleCore/TLTime.h>
#include "TScenegraph.h"

using namespace TLScene;

TSceneNode_Emitter::TSceneNode_Emitter(TRef refNodeID,TRefRef TypeRef) :
  TSceneNode_Object(refNodeID,TypeRef),
  m_fEmissionRate(1.0f),
  m_fEmissionTime(0.0f)
{

}


void TSceneNode_Emitter::ProcessMessage(TPtr<TLMessaging::TMessage>& pMessage)
{
	if(pMessage->GetMessageRef() == "SETPROPERTIES")
	{
		// Set position
		float3 vPos;

		if(pMessage->ImportData("POS", vPos))
		{
			SetPosition(vPos);
		}

		TRef refNodeType;
		if(pMessage->ImportData("TYPE", refNodeType))
		{
			m_refNodeTypeToEmit = refNodeType;
		}

		if(pMessage->ImportData("NAME", refNodeType))
		{
			m_refNodeIDBase = refNodeType;
			m_refNodeIDCurrent = m_refNodeIDBase;
		}

		float fRate = 0.0f; 
		if(pMessage->ImportData("RATE", fRate))
		{
			// Invert the rate passed in due to how we calculate the amount in the update
			// Eg if we have a rate of 3 that's interpreted as 3 per second so due to time being
			// passed in seconds we need to use the inverted value as the actual rate in use
			// and we also then avoid having to calculate this during the update.
			m_fEmissionRate = 1.0f / fRate;	
		}

		if(pMessage->ImportData("MESH", refNodeType))
			m_refMeshToEmitFrom = refNodeType;
	}


	// Super class process message 
	TSceneNode_Object::ProcessMessage(pMessage);
}


void TSceneNode_Emitter::DoUpdate(float fTimestep)
{
	// Check to see if we have an object specified for emission.  
	// If not then return.
	if(!m_refNodeTypeToEmit.IsValid())
		return;

	//Don't emit anything if the timestep is almost zero
	if(fTimestep < 0.001f)
		return;

	// Increase the internal timer
	m_fEmissionTime += 1.f/TLTime::GetUpdatesPerSecond();	//fTimestep;

	// Determine if we need to emit an object
	while(m_fEmissionTime > m_fEmissionRate)
	{
		// Emit object
		EmitObject();

		//TODO: It would be nice to have a facility to be able to setup all of the nodes protperties
		// somehow via the emitter.  One way could be to reference a 'template' object that exists
		// but is not part of the scenegraph (or something?) and then clone it
	
		// Reduce the emission internal timer by the emission rate
		// if we need to emit further objects that will be done due to the while loop
		m_fEmissionTime -= m_fEmissionRate;
	}
}


Bool TSceneNode_Emitter::EmitObject()
{
	// Create a new object via the scenegraph
	TPtr<TSceneNode> pNode = TLScene::g_pScenegraph->CreateInstance(m_refNodeIDCurrent , m_refNodeTypeToEmit);

	if(pNode)
	{
		// Send a property set message
		TPtr<TLMessaging::TMessage> pMessage = new TLMessaging::TMessage("SETPROPERTIES");

		if(pMessage)
		{
			float3 vPos = GetEmissionPosition(); 
			pMessage->AddChildAndData("POS", vPos);

			pNode->QueueMessage(pMessage);
		}

		// Send it an initialise message
		pMessage = new TLMessaging::TMessage(TLCore::InitialiseRef);

		if(pMessage)
			pNode->QueueMessage(pMessage);

		m_refNodeIDCurrent.Increment();

		// Find parent node to add the emitter to

		// Add to scenegraph
		return TLScene::g_pScenegraph->AddNode(pNode);
	}

	return FALSE;
}


float3 TSceneNode_Emitter::GetEmissionPosition()
{
	float3 vPos = GetPosition();

	if(m_refMeshToEmitFrom.IsValid())
	{
		// Find the mesh to reference
		TPtr<TLAsset::TMesh> pMesh = TLAsset::GetAsset( m_refMeshToEmitFrom, "Mesh");

		if(pMesh)
		{
			// If we find a valid mesh, get a random vertex to emit from
			const TArray<float3>& VertexArray = pMesh->GetVertexes();

			if(VertexArray.GetSize())
			{
				// More than one vertex?
				if(VertexArray.GetSize() == 1)
				{
					// Get the vertex position
					vPos = VertexArray[0];
				}
				else
				{
					// Get random vertex and a neighboring vertex
					u32 uIndex = TLMaths::Rand(VertexArray.GetSize());
					u32 uIndex2 = uIndex;

					// get specific indices of neighbouring vertices
					if(uIndex == 0)
						uIndex2 = 1;	// Second index
					else if(uIndex == VertexArray.LastIndex())
						uIndex2 = VertexArray.LastIndex() - 1;		// Second to last index
					else
					{
						// Pick either the one before or after randomly
						u32 uOffset = (TLMaths::Rand(100) > 50 ? -1 : 1);
						uIndex2 =  uIndex + uOffset;
					}
	
					// get the vertices
					float3 vVertexAPos = VertexArray[uIndex];
					float3 vVertexBPos = VertexArray[uIndex2];

					// Now pick a random point between the two verts
					float fScale = (float)TLMaths::Rand(0,100) / 100.0f;
					float3 vDelta = vVertexBPos - vVertexAPos;
					vPos = vVertexAPos + (vDelta * fScale);
				}
			}
		}

	}

	return vPos;
}
